pub mod config;
pub mod diagnostics;
pub mod utils;

use config::MullConfigSpec;

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

// TODO: deduplicate
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
