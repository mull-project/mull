pub mod config;
pub mod diagnostics;
pub mod utils;

use config::MullConfigSpec;

use clap::{CommandFactory, FromArgMatches};
use std::env;
use std::fs;

pub use diagnostics::MullDiagnostics;

// Runtime config types (used internally and converted to FFI types by mull-cxx-bridge)

#[derive(Debug, Clone, Default)]
pub struct DebugOptions {
    pub print_ir: bool,
    pub print_ir_before: bool,
    pub print_ir_after: bool,
    pub print_ir_to_file: bool,
    pub trace_mutants: bool,
    pub coverage: bool,
    pub git_diff: bool,
    pub filters: bool,
    pub slow_ir_verification: bool,
}

#[derive(Debug, Clone)]
pub struct MullConfig {
    pub timeout: u32,
    pub workers: u32,
    pub execution_workers: u32,

    pub debug_enabled: bool,
    pub quiet: bool,
    pub silent: bool,
    pub strict: bool,
    pub dry_run_enabled: bool,
    pub capture_test_output: bool,
    pub capture_mutant_output: bool,
    pub include_not_covered: bool,
    pub junk_detection_disabled: bool,

    pub compilation_database_path: String,
    pub git_diff_ref: String,
    pub git_project_root: String,

    pub mutators: Vec<String>,
    pub ignore_mutators: Vec<String>,
    pub compiler_flags: Vec<String>,
    pub include_paths: Vec<String>,
    pub exclude_paths: Vec<String>,

    pub debug: DebugOptions,
    pub config_path: String,
}

#[derive(Debug, Clone)]
pub struct CliConfig {
    pub input_file: String,
    pub test_program: String,
    pub sqlite_report: String,
    pub runner_args: Vec<String>,

    pub reporters: Vec<String>,
    pub report_name: String,
    pub report_dir: String,
    pub report_patch_base: String,
    pub ide_reporter_show_killed: bool,

    pub strict: bool,
    pub allow_surviving: bool,
    pub mutation_score_threshold: u32,

    pub ld_search_paths: Vec<String>,
    pub coverage_info: String,
}

/// Wrapper for CLI initialization results
pub struct MullCLI {
    diag: MullDiagnostics,
    config: MullConfig,
    cli: CliConfig,
}

impl MullCLI {
    pub fn config_cli(&self) -> &MullConfig {
        &self.config
    }

    pub fn diag_cli(&self) -> &MullDiagnostics {
        &self.diag
    }

    pub fn cli(&self) -> &CliConfig {
        &self.cli
    }

    /// Consume the CLI wrapper and return its components
    pub fn into_parts(self: Box<Self>) -> (MullDiagnostics, MullConfig, CliConfig) {
        (self.diag, self.config, self.cli)
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

fn create_config(diag: &MullDiagnostics) -> MullConfig {
    match get_config_path() {
        Some(path) => match load_yaml_config(&path) {
            Ok(mut config) => {
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

fn init_diagnostics(diag: &MullDiagnostics, config: &MullConfig) {
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

pub fn init_core() -> (MullDiagnostics, MullConfig) {
    let diag = MullDiagnostics::new();
    let config = create_config(&diag);
    init_diagnostics(&diag, &config);
    if !config.config_path.is_empty() {
        diag_info!(diag, "Using config {}", config.config_path);
    }
    (diag, config)
}

impl MullConfigSpec {
    fn to_runtime(&self) -> MullConfig {
        MullConfig {
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
            debug: DebugOptions {
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

fn default_config() -> MullConfig {
    MullConfigSpec::default().to_runtime()
}

fn default_cli_config() -> CliConfig {
    CliConfig {
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

fn parse_yaml(yaml: &str) -> Result<MullConfig, String> {
    let parsed: MullConfigSpec =
        serde_yaml::from_str(yaml).map_err(|e| format!("Failed to parse YAML: {}", e))?;
    Ok(parsed.to_runtime())
}

fn load_yaml_config(config_path: &str) -> Result<MullConfig, String> {
    let contents =
        fs::read_to_string(config_path).map_err(|e| format!("Cannot read config: {}", e))?;
    parse_yaml(&contents)
}

/// Apply SharedCli fields to the yaml config (overrides) and cli config (copies).
fn apply_shared_cli(
    shared: &config::SharedCli,
    yaml_config: &mut MullConfig,
    cli_config: &mut CliConfig,
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

fn normalize_args(args: &[String]) -> Vec<String> {
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

pub fn long_version_string(mull_version: &str, llvm_version: &str) -> &'static str {
    let s = format!(
        "{}\n\n\
         Mull: Practical mutation testing and fault injection for C and C++\n\
         Home: https://github.com/mull-project/mull\n\
         Docs: https://mull.readthedocs.io\n\
         Support: https://mull.readthedocs.io/en/latest/Support.html\n\
         LLVM: {}",
        mull_version, llvm_version
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

pub fn init_runner_cli(
    args: Vec<String>,
    mull_version: String,
    llvm_version: String,
) -> Box<MullCLI> {
    let args = normalize_args(&args);
    let cmd = config::RunnerCli::command()
        .long_version(long_version_string(&mull_version, &llvm_version));
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

pub fn init_reporter_cli(
    args: Vec<String>,
    mull_version: String,
    llvm_version: String,
) -> Box<MullCLI> {
    let args = normalize_args(&args);
    let cmd = config::ReporterCli::command()
        .long_version(long_version_string(&mull_version, &llvm_version));
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
