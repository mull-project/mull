pub mod config;

pub use config::MullConfigSpec;

use clap::{CommandFactory, FromArgMatches};
use std::env;
use std::fs;

#[cfg(test)]
const DEFAULT_TIMEOUT: u32 = 3000;

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
        // Execution
        timeout: u32,
        workers: u32,
        execution_workers: u32,

        // Flags
        debug_enabled: bool,
        quiet: bool,
        silent: bool,
        dry_run_enabled: bool,
        capture_test_output: bool,
        capture_mutant_output: bool,
        include_not_covered: bool,
        junk_detection_disabled: bool,

        // Strings (empty = not set)
        compilation_database_path: String,
        git_diff_ref: String,
        git_project_root: String,

        // Vectors
        mutators: Vec<String>,
        ignore_mutators: Vec<String>,
        compiler_flags: Vec<String>,
        include_paths: Vec<String>,
        exclude_paths: Vec<String>,

        // Nested
        debug: DebugOptions,
    }

    struct LoadConfigResult {
        config: MullConfig,
        error_message: String,
    }

    struct CliConfig {
        config: MullConfig,

        // CLI-only: tool input
        input_file: String,
        test_program: String,
        sqlite_report: String,
        runner_args: Vec<String>,

        // CLI-only: reporting
        reporters: Vec<String>,
        report_name: String,
        report_dir: String,
        report_patch_base: String,
        ide_reporter_show_killed: bool,

        // CLI-only: behavior
        strict: bool,
        allow_surviving: bool,
        mutation_score_threshold: u32,

        // CLI-only: runner
        ld_search_paths: Vec<String>,
        coverage_info: String,

        // Path to the config file that was loaded (empty if none)
        config_path: String,
    }

    struct CliParseResult {
        cli_config: CliConfig,
        error_message: String,
    }

    extern "Rust" {
        fn load_config(path: &str) -> LoadConfigResult;
        fn find_config_path() -> String;
        fn parse_runner_cli(args: Vec<String>) -> CliParseResult;
        fn parse_reporter_cli(args: Vec<String>) -> CliParseResult;
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
        config: default_config(),
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
        config_path: String::new(),
    }
}

fn validate(config: &ffi::MullConfig) -> Vec<String> {
    let mut errors = Vec::new();
    if config.timeout < 100 {
        errors.push(format!(
            "timeout must be >= 100, got {}",
            config.timeout
        ));
    }
    errors
}

fn parse_yaml(yaml: &str) -> Result<ffi::MullConfig, String> {
    let parsed: MullConfigSpec =
        serde_yaml::from_str(yaml).map_err(|e| format!("Failed to parse YAML: {}", e))?;
    let config = parsed.to_ffi();
    let errors = validate(&config);
    if !errors.is_empty() {
        return Err(errors.join("; "));
    }
    Ok(config)
}

fn find_config_path() -> String {
    find_config().unwrap_or_default()
}

fn load_config(path: &str) -> ffi::LoadConfigResult {
    let contents = match fs::read_to_string(path) {
        Ok(c) => c,
        Err(e) => {
            return ffi::LoadConfigResult {
                config: default_config(),
                error_message: format!("Cannot read config: {}", e),
            };
        }
    };
    match parse_yaml(&contents) {
        Ok(config) => ffi::LoadConfigResult {
            config,
            error_message: String::new(),
        },
        Err(e) => ffi::LoadConfigResult {
            config: default_config(),
            error_message: e,
        },
    }
}

// ═══════════════════════════════════════════════════════════════════
// Config file discovery — Rust reimplementation of C++ findConfig
// ═══════════════════════════════════════════════════════════════════

fn find_config() -> Option<String> {
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

fn load_yaml_config(config_path: &str) -> Result<ffi::MullConfig, String> {
    let contents =
        fs::read_to_string(config_path).map_err(|e| format!("Cannot read config: {}", e))?;
    parse_yaml(&contents)
}

// ═══════════════════════════════════════════════════════════════════
// CLI parsing + YAML merge
// ═══════════════════════════════════════════════════════════════════

/// Normalize args so that single-dash long options (e.g. `-reporters`)
/// are converted to double-dash (`--reporters`), matching LLVM cl:: behavior.
/// Leaves the program name (index 0), bare `-`, `--`, and single-char
/// flags like `-v` untouched. Everything after a bare `--` is passed through.
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
        // e.g. "-reporters" → "--reporters", "-timeout=5000" → "--timeout=5000"
        if arg.starts_with('-') && !arg.starts_with("--") && arg.len() > 2 {
            out.push(format!("-{}", arg));
        } else {
            out.push(arg.clone());
        }
    }
    out
}

/// Check if a particular clap argument was explicitly provided on the command line.
fn arg_present(matches: &clap::ArgMatches, id: &str) -> bool {
    matches
        .value_source(id)
        .map(|s| s == clap::parser::ValueSource::CommandLine)
        .unwrap_or(false)
}

fn parse_runner_cli(args: Vec<String>) -> ffi::CliParseResult {
    let args = normalize_args(&args);
    let matches = match config::RunnerCli::command().try_get_matches_from(&args) {
        Ok(m) => m,
        Err(e) => {
            return ffi::CliParseResult {
                cli_config: default_cli_config(),
                error_message: e.to_string(),
            };
        }
    };
    let cli = config::RunnerCli::from_arg_matches(&matches).unwrap();

    // Find and load YAML config
    let config_path = find_config();
    let mut yaml_config = match &config_path {
        Some(path) => match load_yaml_config(path) {
            Ok(c) => c,
            Err(e) => {
                return ffi::CliParseResult {
                    cli_config: default_cli_config(),
                    error_message: e,
                };
            }
        },
        None => default_config(),
    };

    // Merge "both" fields: CLI overrides YAML when explicitly provided
    if arg_present(&matches, "debug") && cli.shared.debug {
        yaml_config.debug_enabled = true;
    }
    if arg_present(&matches, "timeout") {
        if let Some(t) = cli.timeout {
            yaml_config.timeout = t;
        }
    }
    if arg_present(&matches, "workers") {
        if let Some(w) = cli.workers {
            yaml_config.workers = w;
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

    // Inverted flags: override YAML capture settings
    if cli.shared.no_test_output || cli.shared.no_output {
        yaml_config.capture_test_output = false;
    }
    if cli.shared.no_mutant_output || cli.shared.no_output {
        yaml_config.capture_mutant_output = false;
    }

    // Validate merged config
    let errors = validate(&yaml_config);
    if !errors.is_empty() {
        return ffi::CliParseResult {
            cli_config: default_cli_config(),
            error_message: errors.join("; "),
        };
    }

    ffi::CliParseResult {
        cli_config: ffi::CliConfig {
            config: yaml_config,
            input_file: cli.input_file,
            test_program: cli.test_program.unwrap_or_default(),
            sqlite_report: String::new(),
            runner_args: cli.runner_args,
            reporters: cli.shared.reporters,
            report_name: cli.shared.report_name.unwrap_or_default(),
            report_dir: cli.shared.report_dir,
            report_patch_base: cli.shared.report_patch_base,
            ide_reporter_show_killed: cli.shared.ide_reporter_show_killed,
            strict: cli.shared.strict,
            allow_surviving: cli.shared.allow_surviving,
            mutation_score_threshold: cli.shared.mutation_score_threshold,
            ld_search_paths: cli.ld_search_paths,
            coverage_info: cli.coverage_info.unwrap_or_default(),
            config_path: config_path.unwrap_or_default(),
        },
        error_message: String::new(),
    }
}

fn parse_reporter_cli(args: Vec<String>) -> ffi::CliParseResult {
    let args = normalize_args(&args);
    let matches = match config::ReporterCli::command().try_get_matches_from(&args) {
        Ok(m) => m,
        Err(e) => {
            return ffi::CliParseResult {
                cli_config: default_cli_config(),
                error_message: e.to_string(),
            };
        }
    };
    let cli = config::ReporterCli::from_arg_matches(&matches).unwrap();

    // Find and load YAML config
    let config_path = find_config();
    let mut yaml_config = match &config_path {
        Some(path) => match load_yaml_config(path) {
            Ok(c) => c,
            Err(e) => {
                return ffi::CliParseResult {
                    cli_config: default_cli_config(),
                    error_message: e,
                };
            }
        },
        None => default_config(),
    };

    // Merge "both" fields: CLI overrides YAML when explicitly provided
    if arg_present(&matches, "debug") && cli.shared.debug {
        yaml_config.debug_enabled = true;
    }

    // Inverted flags: override YAML capture settings
    if cli.shared.no_test_output || cli.shared.no_output {
        yaml_config.capture_test_output = false;
    }
    if cli.shared.no_mutant_output || cli.shared.no_output {
        yaml_config.capture_mutant_output = false;
    }

    // Validate merged config
    let errors = validate(&yaml_config);
    if !errors.is_empty() {
        return ffi::CliParseResult {
            cli_config: default_cli_config(),
            error_message: errors.join("; "),
        };
    }

    ffi::CliParseResult {
        cli_config: ffi::CliConfig {
            config: yaml_config,
            input_file: String::new(),
            test_program: String::new(),
            sqlite_report: cli.sqlite_report,
            runner_args: Vec::new(),
            reporters: cli.shared.reporters,
            report_name: cli.shared.report_name.unwrap_or_default(),
            report_dir: cli.shared.report_dir,
            report_patch_base: cli.shared.report_patch_base,
            ide_reporter_show_killed: cli.shared.ide_reporter_show_killed,
            strict: cli.shared.strict,
            allow_surviving: cli.shared.allow_surviving,
            mutation_score_threshold: cli.shared.mutation_score_threshold,
            ld_search_paths: Vec::new(),
            coverage_info: String::new(),
            config_path: config_path.unwrap_or_default(),
        },
        error_message: String::new(),
    }
}

#[cfg(test)]
fn load_config_from_yaml(yaml: &str) -> ffi::LoadConfigResult {
    match parse_yaml(yaml) {
        Ok(config) => ffi::LoadConfigResult {
            config,
            error_message: String::new(),
        },
        Err(e) => ffi::LoadConfigResult {
            config: default_config(),
            error_message: e,
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // --- Scalar fields ---

    #[test]
    fn parse_timeout_from_yaml() {
        let config = parse_yaml("timeout: 5000\n").unwrap();
        assert_eq!(config.timeout, 5000);
    }

    #[test]
    fn parse_workers_from_yaml() {
        let config = parse_yaml(
            "parallelization:\n  workers: 4\n  executionWorkers: 8\n",
        )
        .unwrap();
        assert_eq!(config.workers, 4);
        assert_eq!(config.execution_workers, 8);
    }

    #[test]
    fn parse_debug_enabled() {
        let config = parse_yaml("debugEnabled: true\n").unwrap();
        assert!(config.debug_enabled);
    }

    #[test]
    fn parse_quiet() {
        let config = parse_yaml("quiet: false\n").unwrap();
        assert!(!config.quiet);
    }

    #[test]
    fn parse_silent() {
        let config = parse_yaml("silent: true\n").unwrap();
        assert!(config.silent);
    }

    #[test]
    fn parse_dry_run_enabled() {
        let config = parse_yaml("dryRunEnabled: true\n").unwrap();
        assert!(config.dry_run_enabled);
    }

    #[test]
    fn parse_capture_test_output() {
        let config = parse_yaml("captureTestOutput: false\n").unwrap();
        assert!(!config.capture_test_output);
    }

    #[test]
    fn parse_capture_mutant_output() {
        let config = parse_yaml("captureMutantOutput: false\n").unwrap();
        assert!(!config.capture_mutant_output);
    }

    #[test]
    fn parse_include_not_covered() {
        let config = parse_yaml("includeNotCovered: true\n").unwrap();
        assert!(config.include_not_covered);
    }

    #[test]
    fn parse_junk_detection_disabled() {
        let config = parse_yaml("junkDetectionDisabled: true\n").unwrap();
        assert!(config.junk_detection_disabled);
    }

    #[test]
    fn parse_compilation_database_path() {
        let config =
            parse_yaml("compilationDatabasePath: /tmp/compile_commands.json\n").unwrap();
        assert_eq!(config.compilation_database_path, "/tmp/compile_commands.json");
    }

    #[test]
    fn parse_git_diff_ref() {
        let config = parse_yaml("gitDiffRef: main\n").unwrap();
        assert_eq!(config.git_diff_ref, "main");
    }

    #[test]
    fn parse_git_project_root() {
        let config = parse_yaml("gitProjectRoot: /home/user/project\n").unwrap();
        assert_eq!(config.git_project_root, "/home/user/project");
    }

    // --- Vector fields ---

    #[test]
    fn parse_mutators() {
        let config = parse_yaml(
            "mutators:\n  - cxx_add_to_sub\n  - cxx_mul_to_div\n",
        )
        .unwrap();
        assert_eq!(config.mutators, vec!["cxx_add_to_sub", "cxx_mul_to_div"]);
    }

    #[test]
    fn parse_ignore_mutators() {
        let config = parse_yaml(
            "ignoreMutators:\n  - cxx_add_to_sub\n",
        )
        .unwrap();
        assert_eq!(config.ignore_mutators, vec!["cxx_add_to_sub"]);
    }

    #[test]
    fn parse_compiler_flags() {
        let config = parse_yaml(
            "compilerFlags:\n  - -std=c++17\n  - -O2\n",
        )
        .unwrap();
        assert_eq!(config.compiler_flags, vec!["-std=c++17", "-O2"]);
    }

    #[test]
    fn parse_include_paths() {
        let config = parse_yaml(
            "includePaths:\n  - src/\n  - lib/\n",
        )
        .unwrap();
        assert_eq!(config.include_paths, vec!["src/", "lib/"]);
    }

    #[test]
    fn parse_exclude_paths() {
        let config = parse_yaml(
            "excludePaths:\n  - third_party/\n",
        )
        .unwrap();
        assert_eq!(config.exclude_paths, vec!["third_party/"]);
    }

    // --- Nested debug struct ---

    #[test]
    fn parse_debug_options() {
        let yaml = "\
debug:
  printIR: true
  printIRBefore: true
  printIRAfter: true
  printIRToFile: true
  traceMutants: true
  coverage: true
  gitDiff: true
  filters: true
  slowIrVerification: true
";
        let config = parse_yaml(yaml).unwrap();
        assert!(config.debug.print_ir);
        assert!(config.debug.print_ir_before);
        assert!(config.debug.print_ir_after);
        assert!(config.debug.print_ir_to_file);
        assert!(config.debug.trace_mutants);
        assert!(config.debug.coverage);
        assert!(config.debug.git_diff);
        assert!(config.debug.filters);
        assert!(config.debug.slow_ir_verification);
    }

    #[test]
    fn debug_options_default_to_false() {
        let config = parse_yaml("---\n").unwrap();
        assert!(!config.debug.print_ir);
        assert!(!config.debug.print_ir_before);
        assert!(!config.debug.print_ir_after);
        assert!(!config.debug.print_ir_to_file);
        assert!(!config.debug.trace_mutants);
        assert!(!config.debug.coverage);
        assert!(!config.debug.git_diff);
        assert!(!config.debug.filters);
        assert!(!config.debug.slow_ir_verification);
    }

    // --- Defaults ---

    #[test]
    fn default_timeout_when_missing() {
        let config = parse_yaml("mutators:\n  - cxx_add_to_sub\n").unwrap();
        assert_eq!(config.timeout, DEFAULT_TIMEOUT);
    }

    #[test]
    fn empty_yaml_uses_defaults() {
        let config = parse_yaml("---\n").unwrap();
        assert_eq!(config.timeout, DEFAULT_TIMEOUT);
        assert!(config.quiet);
        assert!(config.capture_test_output);
        assert!(config.capture_mutant_output);
        assert!(!config.debug_enabled);
        assert!(!config.silent);
        assert!(!config.dry_run_enabled);
        assert!(!config.include_not_covered);
        assert!(!config.junk_detection_disabled);
        assert_eq!(config.workers, 0);
        assert_eq!(config.execution_workers, 0);
        assert!(config.compilation_database_path.is_empty());
        assert!(config.git_diff_ref.is_empty());
        assert!(config.git_project_root.is_empty());
        assert!(config.mutators.is_empty());
        assert!(config.ignore_mutators.is_empty());
        assert!(config.compiler_flags.is_empty());
        assert!(config.include_paths.is_empty());
        assert!(config.exclude_paths.is_empty());
    }

    // --- camelCase field names ---

    #[test]
    fn camel_case_field_names_work() {
        let yaml = "\
debugEnabled: true
captureTestOutput: false
captureMutantOutput: false
dryRunEnabled: true
includeNotCovered: true
junkDetectionDisabled: true
compilationDatabasePath: /tmp/db.json
gitDiffRef: develop
gitProjectRoot: /home/user
";
        let config = parse_yaml(yaml).unwrap();
        assert!(config.debug_enabled);
        assert!(!config.capture_test_output);
        assert!(!config.capture_mutant_output);
        assert!(config.dry_run_enabled);
        assert!(config.include_not_covered);
        assert!(config.junk_detection_disabled);
        assert_eq!(config.compilation_database_path, "/tmp/db.json");
        assert_eq!(config.git_diff_ref, "develop");
        assert_eq!(config.git_project_root, "/home/user");
    }

    // --- Forward compatibility ---

    #[test]
    fn unknown_fields_are_ignored() {
        let yaml = "\
timeout: 5000
unknownField: some_value
anotherUnknown:
  nested: true
";
        let config = parse_yaml(yaml).unwrap();
        assert_eq!(config.timeout, 5000);
    }

    // --- Error cases ---

    #[test]
    fn invalid_yaml_returns_error() {
        let result = load_config_from_yaml("{{invalid yaml");
        assert!(!result.error_message.is_empty());
        assert!(result.error_message.contains("Failed to parse YAML"));
    }

    #[test]
    fn load_config_reports_error_for_missing_file() {
        let result = load_config("/nonexistent/mull.yml");
        assert!(!result.error_message.is_empty());
        assert_eq!(result.config.timeout, DEFAULT_TIMEOUT);
    }

    #[test]
    fn load_config_success_has_empty_error() {
        let result = load_config_from_yaml("timeout: 7000\n");
        assert!(result.error_message.is_empty());
        assert_eq!(result.config.timeout, 7000);
    }

    // --- Validation ---

    #[test]
    fn validation_catches_low_timeout() {
        let result = load_config_from_yaml("timeout: 50\n");
        assert!(!result.error_message.is_empty());
        assert!(result.error_message.contains("timeout must be >= 100"));
    }

    #[test]
    fn validation_allows_timeout_at_boundary() {
        let config = parse_yaml("timeout: 100\n").unwrap();
        assert_eq!(config.timeout, 100);
    }

    // --- Full config round-trip ---

    #[test]
    fn full_config_round_trip() {
        let yaml = "\
timeout: 10000
debugEnabled: true
quiet: false
silent: true
dryRunEnabled: true
captureTestOutput: false
captureMutantOutput: false
includeNotCovered: true
junkDetectionDisabled: true
compilationDatabasePath: /tmp/compile_commands.json
gitDiffRef: main
gitProjectRoot: /home/user/project
mutators:
  - cxx_add_to_sub
  - cxx_mul_to_div
ignoreMutators:
  - cxx_remove_void
compilerFlags:
  - -std=c++17
includePaths:
  - src/
excludePaths:
  - third_party/
parallelization:
  workers: 4
  executionWorkers: 8
debug:
  printIR: true
  traceMutants: true
  coverage: true
";
        let config = parse_yaml(yaml).unwrap();
        assert_eq!(config.timeout, 10000);
        assert!(config.debug_enabled);
        assert!(!config.quiet);
        assert!(config.silent);
        assert!(config.dry_run_enabled);
        assert!(!config.capture_test_output);
        assert!(!config.capture_mutant_output);
        assert!(config.include_not_covered);
        assert!(config.junk_detection_disabled);
        assert_eq!(config.compilation_database_path, "/tmp/compile_commands.json");
        assert_eq!(config.git_diff_ref, "main");
        assert_eq!(config.git_project_root, "/home/user/project");
        assert_eq!(config.mutators, vec!["cxx_add_to_sub", "cxx_mul_to_div"]);
        assert_eq!(config.ignore_mutators, vec!["cxx_remove_void"]);
        assert_eq!(config.compiler_flags, vec!["-std=c++17"]);
        assert_eq!(config.include_paths, vec!["src/"]);
        assert_eq!(config.exclude_paths, vec!["third_party/"]);
        assert_eq!(config.workers, 4);
        assert_eq!(config.execution_workers, 8);
        assert!(config.debug.print_ir);
        assert!(!config.debug.print_ir_before);
        assert!(config.debug.trace_mutants);
        assert!(config.debug.coverage);
        assert!(!config.debug.filters);
    }

    // --- CLI parsing tests ---

    #[test]
    fn parse_runner_cli_basic() {
        let args = vec![
            "mull-runner".to_string(),
            "/tmp/test".to_string(),
        ];
        let result = parse_runner_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.input_file, "/tmp/test");
    }

    #[test]
    fn parse_runner_cli_with_options() {
        let args = vec![
            "mull-runner".to_string(),
            "--debug".to_string(),
            "--timeout".to_string(),
            "5000".to_string(),
            "--workers".to_string(),
            "4".to_string(),
            "--strict".to_string(),
            "--reporters".to_string(),
            "IDE".to_string(),
            "--reporters".to_string(),
            "SQLite".to_string(),
            "--report-dir".to_string(),
            "/tmp/reports".to_string(),
            "--ld-search-path".to_string(),
            "/usr/lib".to_string(),
            "/tmp/test".to_string(),
        ];
        let result = parse_runner_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.input_file, "/tmp/test");
        assert!(result.cli_config.strict);
        assert_eq!(result.cli_config.reporters, vec!["IDE", "SQLite"]);
        assert_eq!(result.cli_config.report_dir, "/tmp/reports");
        assert_eq!(result.cli_config.ld_search_paths, vec!["/usr/lib"]);
    }

    #[test]
    fn parse_runner_cli_missing_input() {
        let args = vec!["mull-runner".to_string()];
        let result = parse_runner_cli(args);
        assert!(!result.error_message.is_empty());
    }

    #[test]
    fn parse_reporter_cli_basic() {
        let args = vec![
            "mull-reporter".to_string(),
            "/tmp/report.sqlite".to_string(),
        ];
        let result = parse_reporter_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.sqlite_report, "/tmp/report.sqlite");
    }

    #[test]
    fn parse_reporter_cli_with_options() {
        let args = vec![
            "mull-reporter".to_string(),
            "--debug".to_string(),
            "--strict".to_string(),
            "--reporters".to_string(),
            "Elements".to_string(),
            "/tmp/report.sqlite".to_string(),
        ];
        let result = parse_reporter_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.sqlite_report, "/tmp/report.sqlite");
        assert!(result.cli_config.strict);
        assert_eq!(result.cli_config.reporters, vec!["Elements"]);
    }

    #[test]
    fn parse_reporter_cli_missing_input() {
        let args = vec!["mull-reporter".to_string()];
        let result = parse_reporter_cli(args);
        assert!(!result.error_message.is_empty());
    }

    #[test]
    fn parse_runner_cli_no_output_flag() {
        let args = vec![
            "mull-runner".to_string(),
            "--no-output".to_string(),
            "/tmp/test".to_string(),
        ];
        let result = parse_runner_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert!(!result.cli_config.config.capture_test_output);
        assert!(!result.cli_config.config.capture_mutant_output);
    }

    #[test]
    fn parse_runner_cli_runner_args() {
        let args = vec![
            "mull-runner".to_string(),
            "/tmp/test".to_string(),
            "--".to_string(),
            "--gtest_filter=Foo*".to_string(),
            "-v".to_string(),
        ];
        let result = parse_runner_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.runner_args, vec!["--gtest_filter=Foo*", "-v"]);
    }

    // --- normalize_args tests ---

    #[test]
    fn normalize_args_converts_single_dash_long_opts() {
        let args: Vec<String> = vec![
            "mull-runner", "-reporters", "IDE", "-report-dir", "/tmp",
        ].into_iter().map(String::from).collect();
        let normalized = normalize_args(&args);
        assert_eq!(normalized, vec![
            "mull-runner", "--reporters", "IDE", "--report-dir", "/tmp",
        ]);
    }

    #[test]
    fn normalize_args_preserves_double_dash_opts() {
        let args: Vec<String> = vec![
            "mull-runner", "--reporters", "IDE", "--debug",
        ].into_iter().map(String::from).collect();
        let normalized = normalize_args(&args);
        assert_eq!(normalized, vec![
            "mull-runner", "--reporters", "IDE", "--debug",
        ]);
    }

    #[test]
    fn normalize_args_preserves_short_flags() {
        let args: Vec<String> = vec![
            "mull-runner", "-v",
        ].into_iter().map(String::from).collect();
        let normalized = normalize_args(&args);
        assert_eq!(normalized, vec!["mull-runner", "-v"]);
    }

    #[test]
    fn normalize_args_stops_at_double_dash() {
        let args: Vec<String> = vec![
            "mull-runner", "-reporters", "IDE", "--", "-some-test-arg",
        ].into_iter().map(String::from).collect();
        let normalized = normalize_args(&args);
        assert_eq!(normalized, vec![
            "mull-runner", "--reporters", "IDE", "--", "-some-test-arg",
        ]);
    }

    #[test]
    fn normalize_args_handles_equals_syntax() {
        let args: Vec<String> = vec![
            "mull-runner", "-timeout=5000",
        ].into_iter().map(String::from).collect();
        let normalized = normalize_args(&args);
        assert_eq!(normalized, vec!["mull-runner", "--timeout=5000"]);
    }

    #[test]
    fn parse_runner_cli_single_dash_options() {
        let args: Vec<String> = vec![
            "mull-runner", "-debug", "-timeout", "5000", "-reporters", "IDE",
            "-report-dir", "/tmp/reports", "/tmp/test",
        ].into_iter().map(String::from).collect();
        let result = parse_runner_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.input_file, "/tmp/test");
        assert_eq!(result.cli_config.reporters, vec!["IDE"]);
        assert_eq!(result.cli_config.report_dir, "/tmp/reports");
    }

    #[test]
    fn parse_reporter_cli_single_dash_options() {
        let args: Vec<String> = vec![
            "mull-reporter", "-debug", "-strict", "-reporters", "Elements",
            "/tmp/report.sqlite",
        ].into_iter().map(String::from).collect();
        let result = parse_reporter_cli(args);
        assert!(result.error_message.is_empty(), "error: {}", result.error_message);
        assert_eq!(result.cli_config.sqlite_report, "/tmp/report.sqlite");
        assert!(result.cli_config.strict);
        assert_eq!(result.cli_config.reporters, vec!["Elements"]);
    }

    #[test]
    fn parse_runner_cli_defaults() {
        let args = vec![
            "mull-runner".to_string(),
            "/tmp/test".to_string(),
        ];
        let result = parse_runner_cli(args);
        assert!(result.error_message.is_empty());
        assert_eq!(result.cli_config.report_dir, ".");
        assert_eq!(result.cli_config.report_patch_base, ".");
        assert_eq!(result.cli_config.mutation_score_threshold, 100);
        assert!(!result.cli_config.strict);
        assert!(!result.cli_config.allow_surviving);
        assert!(!result.cli_config.ide_reporter_show_killed);
    }
}
