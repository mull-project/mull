use clap::{Args, Parser};
use schemars::JsonSchema;
use serde::{Deserialize, Serialize};

/// YAML configuration for mull.yml files.
#[derive(Debug, Clone, Deserialize, Serialize, JsonSchema)]
#[serde(default, rename_all = "camelCase")]
pub struct MullConfigSpec {
    /// Timeout per test run in milliseconds. DEPRECATED, use CLI flag instead.
    pub timeout: u32,
    /// Enable debug mode with additional diagnostic output.
    pub debug_enabled: bool,
    /// Include mutants on lines not covered by tests. DEPRECATED, use CLI flag instead.
    pub include_not_covered: bool,
    /// Skip mutant execution, only discover and report mutants. DEPRECATED, use CLI flag instead.
    pub dry_run_enabled: bool,

    /// Suppress informational messages (enabled by default).
    pub quiet: bool,
    /// Suppress all output except errors.
    pub silent: bool,
    /// Turn warnings into errors.
    pub strict: bool,
    /// Capture stdout/stderr from the original test run (enabled by default).
    pub capture_test_output: bool,
    /// Capture stdout/stderr from mutant test runs (enabled by default).
    pub capture_mutant_output: bool,

    /// Mutator IDs or groups to enable (e.g. cxx_add_to_sub, cxx_logical).
    pub mutators: Vec<String>,
    /// Mutator IDs or groups to exclude (e.g. cxx_add_to_sub, cxx_logical).
    pub ignore_mutators: Vec<String>,

    /// Path to compilation database (compile_commands.json).
    pub compilation_database_path: String,
    /// Additional compiler flags for junk detection.
    pub compiler_flags: Vec<String>,
    /// Disable junk detection (AST-based mutant validation).
    pub junk_detection_disabled: bool,

    /// File path regex patterns to include in mutation testing.
    pub include_paths: Vec<String>,
    /// File path regex patterns to exclude from mutation testing.
    pub exclude_paths: Vec<String>,
    /// Git ref to diff against for incremental mutation testing.
    pub git_diff_ref: String,
    /// Root directory of the git project.
    pub git_project_root: String,

    /// Parallelization settings.
    pub parallelization: ParallelizationSpec,
    /// Advanced debug options.
    pub debug: DebugOptionsSpec,
}

impl Default for MullConfigSpec {
    fn default() -> Self {
        MullConfigSpec {
            timeout: 3000,
            debug_enabled: false,
            include_not_covered: false,
            dry_run_enabled: false,
            quiet: true,
            silent: false,
            strict: false,
            capture_test_output: true,
            capture_mutant_output: true,
            mutators: Vec::new(),
            ignore_mutators: Vec::new(),
            compilation_database_path: String::new(),
            compiler_flags: Vec::new(),
            junk_detection_disabled: false,
            include_paths: Vec::new(),
            exclude_paths: Vec::new(),
            git_diff_ref: String::new(),
            git_project_root: String::new(),
            parallelization: ParallelizationSpec::default(),
            debug: DebugOptionsSpec::default(),
        }
    }
}

/// Parallelization settings
#[derive(Debug, Clone, Deserialize, Serialize, JsonSchema)]
#[serde(default, rename_all = "camelCase")]
pub struct ParallelizationSpec {
    /// Number of worker threads for test discovery.
    pub workers: u32,
    /// Number of worker threads for mutant execution.
    pub execution_workers: u32,
}

impl Default for ParallelizationSpec {
    fn default() -> Self {
        let max_workers = std::thread::available_parallelism()
            .map(|n| n.get() as u32)
            .unwrap_or(1);
        ParallelizationSpec {
            workers: max_workers,
            execution_workers: max_workers,
        }
    }
}

/// Advanced debug options for diagnostic output.
#[derive(Debug, Clone, Default, Deserialize, Serialize, JsonSchema)]
#[serde(default, rename_all = "camelCase")]
pub struct DebugOptionsSpec {
    /// Print LLVM IR.
    #[serde(rename = "printIR")]
    pub print_ir: bool,
    /// Print LLVM IR before mutation.
    #[serde(rename = "printIRBefore")]
    pub print_ir_before: bool,
    /// Print LLVM IR after mutation.
    #[serde(rename = "printIRAfter")]
    pub print_ir_after: bool,
    /// Print LLVM IR to a file.
    #[serde(rename = "printIRToFile")]
    pub print_ir_to_file: bool,
    /// Trace mutant discovery and execution.
    pub trace_mutants: bool,
    /// Enable coverage debug output.
    #[serde(alias = "coverage")]
    pub coverage: bool,
    /// Enable git diff debug output.
    pub git_diff: bool,
    /// Enable filter debug output.
    pub filters: bool,
    /// Enable slow IR verification.
    #[serde(rename = "slowIRVerification")]
    pub slow_ir_verification: bool,
}

// ═══════════════════════════════════════════════════════════════════
// CLI structs — clap-based argument parsing for mull-runner/reporter
// ═══════════════════════════════════════════════════════════════════

/// CLI options shared between mull-runner and mull-reporter.
#[derive(Debug, Clone, Args)]
pub struct SharedCli {
    /// Output reporters to use (IDE, SQLite, Elements, Patches, GithubAnnotations).
    #[arg(long, value_name = "REPORTER")]
    pub reporters: Vec<String>,

    /// Directory for report output files.
    #[arg(long = "report-dir", value_name = "DIR", default_value = ".")]
    pub report_dir: String,

    /// Filename for the report (only for supported reporters).
    #[arg(long = "report-name", value_name = "NAME")]
    pub report_name: Option<String>,

    /// Base directory for patch file paths.
    #[arg(long = "report-patch-base", value_name = "DIR", default_value = ".")]
    pub report_patch_base: String,

    /// Show killed mutations in IDE reporter output.
    #[arg(long = "ide-reporter-show-killed")]
    pub ide_reporter_show_killed: bool,

    /// Enable debug mode with additional diagnostic output.
    #[arg(long = "debug")]
    pub debug: bool,

    /// Treat warnings as fatal errors.
    #[arg(long)]
    pub strict: bool,

    /// Do not treat surviving mutants as an error.
    #[arg(long = "allow-surviving")]
    pub allow_surviving: bool,

    /// Minimum mutation score (0-100) required for success.
    #[arg(
        long = "mutation-score-threshold",
        value_name = "SCORE",
        default_value_t = 100
    )]
    pub mutation_score_threshold: u32,

    /// Does not capture output from test runs.
    #[arg(long = "no-test-output")]
    pub no_test_output: bool,

    /// Does not capture output from mutant runs.
    #[arg(long = "no-mutant-output")]
    pub no_mutant_output: bool,

    /// Combines --no-test-output and --no-mutant-output.
    #[arg(long = "no-output")]
    pub no_output: bool,
}

/// Mull: Practical mutation testing and fault injection for C and C++
#[derive(Debug, Clone, Parser)]
#[command(name = "mull-runner")]
#[command(about = "Mull: Practical mutation testing and fault injection for C and C++")]
#[command(version)]
pub struct RunnerCli {
    /// Path to the executable under test.
    #[arg(value_name = "EXECUTABLE")]
    pub input_file: String,

    /// Path to a test program (if different from input executable).
    #[arg(long = "test-program", value_name = "PATH")]
    pub test_program: Option<String>,

    /// Number of parallel workers for mutation search.
    #[arg(long, value_name = "N")]
    pub workers: Option<u32>,

    /// Timeout per test run in milliseconds.
    #[arg(long, value_name = "MS")]
    pub timeout: Option<u32>,

    /// Include mutants on lines not covered by tests.
    #[arg(long = "include-not-covered")]
    pub include_not_covered: bool,

    /// Skip mutant execution, only discover and report mutants.
    #[arg(long = "dry-run")]
    pub dry_run: bool,

    /// Library search path.
    #[arg(long = "ld-search-path", value_name = "DIR")]
    pub ld_search_paths: Vec<String>,

    /// Path to the coverage info file (LLVM profdata).
    #[arg(long = "coverage-info", value_name = "PATH")]
    pub coverage_info: Option<String>,

    /// Print coverage ranges.
    #[arg(long = "debug-coverage")]
    pub debug_coverage: bool,

    #[command(flatten)]
    pub shared: SharedCli,

    /// Arguments passed to the test runner.
    #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
    pub runner_args: Vec<String>,
}

/// Mull: mutation testing report generator
#[derive(Debug, Clone, Parser)]
#[command(name = "mull-reporter")]
#[command(about = "Mull: mutation testing report generator")]
#[command(version)]
pub struct ReporterCli {
    /// Path to the sqlite report.
    #[arg(value_name = "SQLITE-REPORT")]
    pub sqlite_report: String,

    #[command(flatten)]
    pub shared: SharedCli,
}
