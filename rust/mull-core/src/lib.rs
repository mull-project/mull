pub mod config;
pub mod diagnostics;

use config::MullConfigSpec;

use clap::{CommandFactory, FromArgMatches};
use std::env;
use std::fs;

pub use diagnostics::MullDiagnostics;

struct MullCore {
    diag: MullDiagnostics,
    config: ffi::MullConfig,
}

struct MullCLI {
    diag: MullDiagnostics,
    config: ffi::MullConfig,
    cli: ffi::CliConfig,
}

#[cxx::bridge]
mod ffi {
    struct DebugOptions {
        print_ir: bool,
        print_ir_before: bool,
        print_ir_after: bool,
        print_ir_to_file: bool,
        trace_mutants: bool,
        coverage: bool,
        git_diff: bool,
        filters: bool,
        slow_ir_verification: bool,
    }

    struct MullConfig {
        timeout: u32,
        workers: u32,
        execution_workers: u32,

        debug_enabled: bool,
        quiet: bool,
        silent: bool,
        strict: bool,
        dry_run_enabled: bool,
        capture_test_output: bool,
        capture_mutant_output: bool,
        include_not_covered: bool,
        junk_detection_disabled: bool,

        compilation_database_path: String,
        git_diff_ref: String,
        git_project_root: String,

        mutators: Vec<String>,
        ignore_mutators: Vec<String>,
        compiler_flags: Vec<String>,
        include_paths: Vec<String>,
        exclude_paths: Vec<String>,

        debug: DebugOptions,
        config_path: String,
    }

    struct CliConfig {
        input_file: String,
        test_program: String,
        sqlite_report: String,
        runner_args: Vec<String>,

        reporters: Vec<String>,
        report_name: String,
        report_dir: String,
        report_patch_base: String,
        ide_reporter_show_killed: bool,

        strict: bool,
        allow_surviving: bool,
        mutation_score_threshold: u32,

        ld_search_paths: Vec<String>,
        coverage_info: String,
    }

    extern "Rust" {
        type MullDiagnostics;

        fn enable_debug_mode(self: &MullDiagnostics);

        fn info(self: &MullDiagnostics, message: &str);
        fn warning(self: &MullDiagnostics, message: &str);
        fn error(self: &MullDiagnostics, message: &str);
        fn debug(self: &MullDiagnostics, message: &str);
        fn progress(self: &MullDiagnostics, message: &str);

        type MullCore;

        fn init_core_ffi() -> Box<MullCore>;
        fn config(self: &MullCore) -> &MullConfig;
        fn diag(self: &MullCore) -> &MullDiagnostics;

        type MullCLI;
        fn init_reporter_cli(args: Vec<String>, llvm_version: String) -> Box<MullCLI>;
        fn init_runner_cli(args: Vec<String>, llvm_version: String) -> Box<MullCLI>;
        fn config_cli(self: &MullCLI) -> &MullConfig;
        fn diag_cli(self: &MullCLI) -> &MullDiagnostics;
        fn cli(self: &MullCLI) -> &CliConfig;
    }
}

pub fn get_config_path() -> Option<String> {
    if let Ok(path) = env::var("MULL_CONFIG") {
        if !path.is_empty() {
            return Some(path);
        }
    }
    let cwd = env::current_dir().ok()?;
    let mut dir = cwd.as_path();
    loop {
        let candidate = dir.join("mull.yml");
        if candidate.exists() {
            return Some(candidate.to_string_lossy().into_owned());
        }
        match dir.parent() {
            Some(parent) => dir = parent,
            None => break,
        }
    }
    None
}

fn create_config(diag: &MullDiagnostics) -> ffi::MullConfig {
    match get_config_path() {
        Some(path) => match load_yaml_config(&path) {
            Ok(mut config) => {
                diag_info!(diag, "Using config {}", path);
                config.config_path = path;
                config
            }
            Err(e) => {
                diag_warning!(
                    diag,
                    "Failed to read config: {}: {}. Using some defaults.",
                    path,
                    e
                );
                default_config()
            }
        },
        None => {
            diag_warning!(
                diag,
                "Mull cannot find config (mull.yml). Using some defaults."
            );
            default_config()
        }
    }
}

fn init_diagnostics(diag: &MullDiagnostics, config: &ffi::MullConfig) {
    if config.debug_enabled {
        diag.enable_debug_mode();
        diag_debug!(
            diag,
            "Diagnostics: Debug Mode enabled. Debug-level messages will be printed."
        );
    }
    if config.strict {
        diag.enable_strict_mode();
        diag_info!(
            diag,
            "Diagnostics: Strict Mode enabled. Warning messages will be treated as fatal errors."
        );
    }
    if config.silent {
        diag.make_silent();
    }
    if config.quiet {
        diag.make_quiet();
    }
}

pub fn init_core() -> (MullDiagnostics, ffi::MullConfig) {
    let diag = MullDiagnostics::new();
    let config = create_config(&diag);
    init_diagnostics(&diag, &config);
    (diag, config)
}

fn init_core_ffi() -> Box<MullCore> {
    let (diag, config) = init_core();
    Box::new(MullCore { diag, config })
}

impl MullCore {
    fn config(&self) -> &ffi::MullConfig {
        &self.config
    }

    fn diag(&self) -> &MullDiagnostics {
        &self.diag
    }
}

impl MullCLI {
    fn config_cli(&self) -> &ffi::MullConfig {
        &self.config
    }

    fn diag_cli(&self) -> &MullDiagnostics {
        &self.diag
    }

    fn cli(&self) -> &ffi::CliConfig {
        &self.cli
    }
}

impl MullConfigSpec {
    fn to_ffi(&self) -> ffi::MullConfig {
        ffi::MullConfig {
            timeout: self.timeout,
            workers: self.parallelization.workers,
            execution_workers: self.parallelization.execution_workers,
            debug_enabled: self.debug_enabled,
            quiet: self.quiet,
            silent: self.silent,
            strict: self.strict,
            dry_run_enabled: self.dry_run_enabled,
            capture_test_output: self.capture_test_output,
            capture_mutant_output: self.capture_mutant_output,
            include_not_covered: self.include_not_covered,
            junk_detection_disabled: self.junk_detection_disabled,
            compilation_database_path: self.compilation_database_path.clone(),
            git_diff_ref: self.git_diff_ref.clone(),
            git_project_root: self.git_project_root.clone(),
            mutators: self.mutators.clone(),
            ignore_mutators: self.ignore_mutators.clone(),
            compiler_flags: self.compiler_flags.clone(),
            include_paths: self.include_paths.clone(),
            exclude_paths: self.exclude_paths.clone(),
            config_path: String::new(),
            debug: ffi::DebugOptions {
                print_ir: self.debug.print_ir,
                print_ir_before: self.debug.print_ir_before,
                print_ir_after: self.debug.print_ir_after,
                print_ir_to_file: self.debug.print_ir_to_file,
                trace_mutants: self.debug.trace_mutants,
                coverage: self.debug.coverage,
                git_diff: self.debug.git_diff,
                filters: self.debug.filters,
                slow_ir_verification: self.debug.slow_ir_verification,
            },
        }
    }
}

fn default_config() -> ffi::MullConfig {
    MullConfigSpec::default().to_ffi()
}

fn default_cli_config() -> ffi::CliConfig {
    ffi::CliConfig {
        input_file: String::new(),
        test_program: String::new(),
        sqlite_report: String::new(),
        runner_args: Vec::new(),
        reporters: Vec::new(),
        report_name: String::new(),
        report_dir: ".".to_string(),
        report_patch_base: ".".to_string(),
        ide_reporter_show_killed: false,
        strict: false,
        allow_surviving: false,
        mutation_score_threshold: 100,
        ld_search_paths: Vec::new(),
        coverage_info: String::new(),
    }
}

fn parse_yaml(yaml: &str) -> Result<ffi::MullConfig, String> {
    let parsed: MullConfigSpec =
        serde_yaml::from_str(yaml).map_err(|e| format!("Failed to parse YAML: {}", e))?;
    Ok(parsed.to_ffi())
}

fn load_yaml_config(config_path: &str) -> Result<ffi::MullConfig, String> {
    let contents =
        fs::read_to_string(config_path).map_err(|e| format!("Cannot read config: {}", e))?;
    parse_yaml(&contents)
}

/// Apply SharedCli fields to the yaml config (overrides) and cli config (copies).
fn apply_shared_cli(
    shared: &config::SharedCli,
    yaml_config: &mut ffi::MullConfig,
    cli_config: &mut ffi::CliConfig,
) {
    // Debug override
    if shared.debug {
        yaml_config.debug_enabled = true;
    }
    // Inverted flags
    if shared.no_test_output || shared.no_output {
        yaml_config.capture_test_output = false;
    }
    if shared.no_mutant_output || shared.no_output {
        yaml_config.capture_mutant_output = false;
    }
    // Copy shared fields
    cli_config.reporters = shared
        .reporters
        .iter()
        .map(|r| format!("{:?}", r))
        .collect();
    cli_config.report_name = shared.report_name.clone().unwrap_or_default();
    cli_config.report_dir = shared.report_dir.clone();
    cli_config.report_patch_base = shared.report_patch_base.clone();
    cli_config.ide_reporter_show_killed = shared.ide_reporter_show_killed;
    cli_config.strict = shared.strict;
    cli_config.allow_surviving = shared.allow_surviving;
    cli_config.mutation_score_threshold = shared.mutation_score_threshold;
}

pub fn normalize_args(args: &[String]) -> Vec<String> {
    let mut out = Vec::with_capacity(args.len());
    let mut passthrough = false;
    for (i, arg) in args.iter().enumerate() {
        if i == 0 || passthrough {
            out.push(arg.clone());
            continue;
        }
        if arg == "--" {
            passthrough = true;
            out.push(arg.clone());
            continue;
        }
        // Single-dash with >1 char after it: convert to double-dash.
        // e.g. "-reporters" -> "--reporters", "-timeout=5000" -> "--timeout=5000"
        if arg.starts_with('-') && !arg.starts_with("--") && arg.len() > 2 {
            out.push(format!("-{}", arg));
        } else {
            out.push(arg.clone());
        }
    }
    out
}

pub fn long_version_string(llvm_version: &str) -> &'static str {
    let s = format!(
        "{}\n\n\
         Mull: Practical mutation testing and fault injection for C and C++\n\
         Home: https://github.com/mull-project/mull\n\
         Docs: https://mull.readthedocs.io\n\
         Support: https://mull.readthedocs.io/en/latest/Support.html\n\
         LLVM: {}",
        env!("CARGO_PKG_VERSION"),
        llvm_version
    );
    // Leak is fine: this runs once per CLI invocation then the process exits.
    Box::leak(s.into_boxed_str())
}

fn arg_present(matches: &clap::ArgMatches, id: &str) -> bool {
    matches
        .value_source(id)
        .map(|s| s == clap::parser::ValueSource::CommandLine)
        .unwrap_or(false)
}

fn init_runner_cli(args: Vec<String>, llvm_version: String) -> Box<MullCLI> {
    let args = normalize_args(&args);
    let cmd = config::RunnerCli::command().long_version(long_version_string(&llvm_version));
    let matches = cmd.get_matches_from(&args);
    let cli = config::RunnerCli::from_arg_matches(&matches).unwrap();

    let diag = MullDiagnostics::new();
    let mut yaml_config = create_config(&diag);
    // defaults to true, but overriding this for CLI tools
    yaml_config.quiet = false;

    // Merge runner-specific "both" fields: CLI overrides YAML when explicitly provided
    if arg_present(&matches, "timeout") {
        if let Some(t) = cli.timeout {
            yaml_config.timeout = t;
        }
    }
    if arg_present(&matches, "workers") {
        if let Some(w) = cli.workers {
            yaml_config.workers = w;
            yaml_config.execution_workers = w;
        }
    }
    if arg_present(&matches, "include_not_covered") && cli.include_not_covered {
        yaml_config.include_not_covered = true;
    }
    if arg_present(&matches, "dry_run") && cli.dry_run {
        yaml_config.dry_run_enabled = true;
    }
    if arg_present(&matches, "debug_coverage") && cli.debug_coverage {
        yaml_config.debug.coverage = true;
    }
    if arg_present(&matches, "strict") && cli.shared.strict {
        yaml_config.strict = true;
    }

    let mut cli_config = default_cli_config();
    apply_shared_cli(&cli.shared, &mut yaml_config, &mut cli_config);
    init_diagnostics(&diag, &yaml_config);

    cli_config.input_file = cli.input_file;
    cli_config.test_program = cli.test_program.unwrap_or_default();
    cli_config.runner_args = cli.runner_args;
    cli_config.ld_search_paths = cli.ld_search_paths;
    cli_config.coverage_info = cli.coverage_info.unwrap_or_default();

    Box::new(MullCLI {
        diag,
        config: yaml_config,
        cli: cli_config,
    })
}

fn init_reporter_cli(args: Vec<String>, llvm_version: String) -> Box<MullCLI> {
    let args = normalize_args(&args);
    let cmd = config::ReporterCli::command().long_version(long_version_string(&llvm_version));
    let matches = cmd.get_matches_from(&args);
    let cli = config::ReporterCli::from_arg_matches(&matches).unwrap();

    let diag = MullDiagnostics::new();
    let mut yaml_config = create_config(&diag);
    // defaults to true, but overriding this for CLI tools
    yaml_config.quiet = false;
    let mut cli_config = default_cli_config();
    apply_shared_cli(&cli.shared, &mut yaml_config, &mut cli_config);
    init_diagnostics(&diag, &yaml_config);

    cli_config.sqlite_report = cli.sqlite_report;

    Box::new(MullCLI {
        diag,
        config: yaml_config,
        cli: cli_config,
    })
}
