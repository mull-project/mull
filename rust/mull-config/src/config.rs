use clap::{Args, Parser};
use serde::Deserialize;

/// Mull: practical mutation testing for C and C++
#[derive(Debug, Clone, Parser, Deserialize)]
#[command(name = "mull-runner")]
#[command(about = "Mull: practical mutation testing for C and C++")]
#[serde(default, rename_all = "camelCase")]
pub struct MullConfigSpec {
    // ─────────────────────────────────────────────────────────────
    // Input/Output — CLI-only (mull-runner)
    // ─────────────────────────────────────────────────────────────
    /// Path to the executable under test.
    #[arg(value_name = "EXECUTABLE")]
    #[serde(skip)]
    pub input_file: Option<String>,

    /// Path to a test program (if different from input executable).
    #[arg(long = "test-program", value_name = "PATH")]
    #[serde(skip)]
    pub test_program: Option<String>,

    /// Arguments passed to the test runner.
    #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
    #[serde(skip)]
    pub runner_args: Vec<String>,

    // ─────────────────────────────────────────────────────────────
    // Execution Control — shared CLI + YAML
    // ─────────────────────────────────────────────────────────────
    /// Number of parallel workers for mutation search.
    #[arg(long, value_name = "N")]
    pub workers: Option<u32>,

    /// Timeout per test run in milliseconds.
    #[arg(long, value_name = "MS", default_value_t = 3000)]
    pub timeout: u32,

    /// Enable debug mode with additional diagnostic output.
    #[arg(long = "debug")]
    pub debug_enabled: bool,

    /// Include mutants on lines not covered by tests.
    #[arg(long = "include-not-covered")]
    pub include_not_covered: bool,

    /// Skip mutant execution, only discover and report mutants.
    #[arg(long = "dry-run")]
    pub dry_run_enabled: bool,

    // ─────────────────────────────────────────────────────────────
    // Output Control — CLI flags (inverted from YAML equivalents)
    // ─────────────────────────────────────────────────────────────
    /// Does not capture output from test runs.
    #[arg(long = "no-test-output")]
    #[serde(skip)]
    pub no_test_output: bool,

    /// Does not capture output from mutant runs.
    #[arg(long = "no-mutant-output")]
    #[serde(skip)]
    pub no_mutant_output: bool,

    /// Combines --no-test-output and --no-mutant-output.
    #[arg(long = "no-output")]
    #[serde(skip)]
    pub no_output: bool,

    // ─────────────────────────────────────────────────────────────
    // Output Control — YAML equivalents (not exposed as CLI flags)
    // ─────────────────────────────────────────────────────────────
    /// Suppress informational messages (enabled by default).
    #[arg(skip)]
    pub quiet: bool,

    /// Suppress all output except errors.
    #[arg(skip)]
    pub silent: bool,

    /// Capture stdout/stderr from the original test run (enabled by default).
    #[arg(skip)]
    pub capture_test_output: bool,

    /// Capture stdout/stderr from mutant test runs (enabled by default).
    #[arg(skip)]
    pub capture_mutant_output: bool,

    // ─────────────────────────────────────────────────────────────
    // Mutators — shared CLI + YAML
    // ─────────────────────────────────────────────────────────────
    /// Mutator IDs or groups to enable (e.g. cxx_add_to_sub, cxx_logical).
    #[arg(long)]
    pub mutators: Vec<String>,

    /// Mutator IDs or groups to exclude.
    #[arg(long = "ignore-mutators")]
    pub ignore_mutators: Vec<String>,

    // ─────────────────────────────────────────────────────────────
    // Junk Detection — YAML-only
    // ─────────────────────────────────────────────────────────────
    /// Path to compilation database (compile_commands.json).
    #[arg(skip)]
    pub compilation_database_path: String,

    /// Additional compiler flags for junk detection.
    #[arg(skip)]
    pub compiler_flags: Vec<String>,

    /// Disable junk detection (AST-based mutant validation).
    #[arg(skip)]
    pub junk_detection_disabled: bool,

    // ─────────────────────────────────────────────────────────────
    // Filtering — YAML-only
    // ─────────────────────────────────────────────────────────────
    /// File path regex patterns to include in mutation testing.
    #[arg(skip)]
    pub include_paths: Vec<String>,

    /// File path regex patterns to exclude from mutation testing.
    #[arg(skip)]
    pub exclude_paths: Vec<String>,

    /// Git ref to diff against for incremental mutation testing.
    #[arg(skip)]
    pub git_diff_ref: String,

    /// Root directory of the git project.
    #[arg(skip)]
    pub git_project_root: String,

    // ─────────────────────────────────────────────────────────────
    // Reporting — shared CLI (runner + reporter)
    // ─────────────────────────────────────────────────────────────
    /// Output reporters to use (IDE, SQLite, Elements, Patches, GithubAnnotations).
    #[arg(long, value_name = "REPORTER")]
    #[serde(skip)]
    pub reporters: Vec<String>,

    /// Directory for report output files.
    #[arg(long = "report-dir", value_name = "DIR", default_value = ".")]
    #[serde(skip)]
    pub report_dir: String,

    /// Filename for the report (only for supported reporters).
    #[arg(long = "report-name", value_name = "NAME")]
    #[serde(skip)]
    pub report_name: Option<String>,

    /// Base directory for patch file paths.
    #[arg(long = "report-patch-base", value_name = "DIR", default_value = ".")]
    #[serde(skip)]
    pub report_patch_base: String,

    /// Show killed mutations in IDE reporter output.
    #[arg(long = "ide-reporter-show-killed")]
    #[serde(skip)]
    pub ide_reporter_show_killed: bool,

    // ─────────────────────────────────────────────────────────────
    // Exit Behavior — shared CLI (runner + reporter)
    // ─────────────────────────────────────────────────────────────
    /// Treat warnings as fatal errors.
    #[arg(long)]
    #[serde(skip)]
    pub strict: bool,

    /// Do not treat surviving mutants as an error.
    #[arg(long = "allow-surviving")]
    #[serde(skip)]
    pub allow_surviving: bool,

    /// Minimum mutation score (0-100) required for success.
    #[arg(
        long = "mutation-score-threshold",
        value_name = "SCORE",
        default_value_t = 100
    )]
    #[serde(skip)]
    pub mutation_score_threshold: u32,

    // ─────────────────────────────────────────────────────────────
    // Runner-only CLI options
    // ─────────────────────────────────────────────────────────────
    /// Library search path.
    #[arg(long = "ld-search-path", value_name = "DIR")]
    #[serde(skip)]
    pub ld_search_paths: Vec<String>,

    /// Path to the coverage info file (LLVM profdata).
    #[arg(long = "coverage-info", value_name = "PATH")]
    #[serde(skip)]
    pub coverage_info: Option<String>,

    // ─────────────────────────────────────────────────────────────
    // Nested — YAML-only
    // ─────────────────────────────────────────────────────────────
    /// Parallelization settings.
    #[command(flatten)]
    pub parallelization: ParallelizationSpec,

    /// Advanced debug options (set via config file).
    #[command(flatten)]
    pub debug: DebugOptionsSpec,
}

impl Default for MullConfigSpec {
    fn default() -> Self {
        MullConfigSpec {
            input_file: None,
            test_program: None,
            runner_args: Vec::new(),
            workers: None,
            timeout: 3000,
            debug_enabled: false,
            include_not_covered: false,
            dry_run_enabled: false,
            no_test_output: false,
            no_mutant_output: false,
            no_output: false,
            quiet: true,
            silent: false,
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
            reporters: Vec::new(),
            report_dir: ".".to_string(),
            report_name: None,
            report_patch_base: ".".to_string(),
            ide_reporter_show_killed: false,
            strict: false,
            allow_surviving: false,
            mutation_score_threshold: 100,
            ld_search_paths: Vec::new(),
            coverage_info: None,
            parallelization: ParallelizationSpec::default(),
            debug: DebugOptionsSpec::default(),
        }
    }
}

/// Parallelization settings for mutation testing.
#[derive(Debug, Clone, Default, Args, Deserialize)]
#[serde(default, rename_all = "camelCase")]
pub struct ParallelizationSpec {
    /// Number of worker threads for test discovery.
    #[arg(skip)]
    pub workers: u32,

    /// Number of worker threads for mutant execution.
    #[arg(skip)]
    pub execution_workers: u32,
}

/// Advanced debug options for diagnostic output.
#[derive(Debug, Clone, Default, Args, Deserialize)]
#[serde(default, rename_all = "camelCase")]
pub struct DebugOptionsSpec {
    /// Print LLVM IR.
    #[arg(skip)]
    #[serde(rename = "printIR")]
    pub print_ir: bool,

    /// Print LLVM IR before mutation.
    #[arg(skip)]
    #[serde(rename = "printIRBefore")]
    pub print_ir_before: bool,

    /// Print LLVM IR after mutation.
    #[arg(skip)]
    #[serde(rename = "printIRAfter")]
    pub print_ir_after: bool,

    /// Print LLVM IR to a file.
    #[arg(skip)]
    #[serde(rename = "printIRToFile")]
    pub print_ir_to_file: bool,

    /// Trace mutant discovery and execution.
    #[arg(skip)]
    pub trace_mutants: bool,

    /// Enable coverage debug output.
    #[arg(long = "debug-coverage")]
    #[serde(alias = "coverage")]
    pub coverage: bool,

    /// Enable git diff debug output.
    #[arg(skip)]
    pub git_diff: bool,

    /// Enable filter debug output.
    #[arg(skip)]
    pub filters: bool,

    /// Enable slow IR verification.
    #[arg(skip)]
    pub slow_ir_verification: bool,
}

/// Option availability across tools and configuration sources.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OptionScope {
    /// Available in mull.yml only.
    YamlOnly,
    /// CLI flag on mull-runner only.
    RunnerCli,
    /// CLI flag on mull-reporter only.
    ReporterCli,
    /// CLI flag on both mull-runner and mull-reporter.
    SharedCli,
    /// Available in both CLI and mull.yml.
    Both,
}

impl OptionScope {
    /// Whether this option appears in the mull-runner CLI.
    pub fn in_runner(self) -> bool {
        matches!(self, Self::RunnerCli | Self::SharedCli | Self::Both)
    }
    /// Whether this option appears in the mull-reporter CLI.
    pub fn in_reporter(self) -> bool {
        matches!(self, Self::ReporterCli | Self::SharedCli | Self::Both)
    }
}

/// Metadata for a single configuration option, used by doc generators.
pub struct OptionMeta {
    pub cli_long: Option<&'static str>,
    pub yaml_key: Option<&'static str>,
    pub help: &'static str,
    pub scope: OptionScope,
    pub default_display: &'static str,
    pub section: &'static str,
    /// Value hint shown in CLI docs (e.g. "path", "number", "reporter").
    /// Empty string means no value (boolean flag).
    pub value_hint: &'static str,
    /// Extra lines appended after the help text (e.g. reporter sub-options).
    pub extra: &'static str,
}

/// Helper to build an OptionMeta with common defaults.
fn opt(
    cli_long: Option<&'static str>,
    yaml_key: Option<&'static str>,
    help: &'static str,
    scope: OptionScope,
    section: &'static str,
) -> OptionMeta {
    OptionMeta {
        cli_long,
        yaml_key,
        help,
        scope,
        default_display: "",
        section,
        value_hint: "",
        extra: "",
    }
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

/// Mull: practical mutation testing for C and C++
#[derive(Debug, Clone, Parser)]
#[command(name = "mull-runner")]
#[command(about = "Mull: practical mutation testing for C and C++")]
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

/// Returns metadata for all configuration options, in display order.
/// Doc generators use this to produce properly categorized output.
pub fn option_metadata() -> Vec<OptionMeta> {
    use OptionScope::*;
    vec![
        // ── mull-runner positional ──
        OptionMeta { value_hint: "path", ..opt(Some("test-program"), None, "Path to a test program", RunnerCli, "Input") },

        // ── mull-reporter positional ──
        OptionMeta { value_hint: "path", ..opt(Some("sqlite-report"), None, "Path to the sqlite report", ReporterCli, "Input") },

        // ── Execution ──
        OptionMeta { value_hint: "number", ..opt(Some("workers"), Some("workers"), "How many threads to use", RunnerCli, "Execution") },
        OptionMeta { value_hint: "number", ..opt(Some("timeout"), Some("timeout"), "Timeout per test run (milliseconds)", RunnerCli, "Execution") },

        // ── Reporting ──
        OptionMeta { value_hint: "filename", ..opt(Some("report-name"), None, "Filename for the report (only for supported reporters). Defaults to <timestamp>.<extension>", SharedCli, "Reporting") },
        OptionMeta { value_hint: "directory", ..opt(Some("report-dir"), None, "Where to store report (defaults to '.')", SharedCli, "Reporting") },
        OptionMeta { value_hint: "directory", ..opt(Some("report-patch-base"), None, "Create Patches relative to this directory (defaults to gitProjectRoot if available, else absolute path will be used)", SharedCli, "Reporting") },
        OptionMeta {
            value_hint: "reporter",
            extra: "\n    :IDE:\tPrints compiler-like warnings into stdout\n\n    :SQLite:\tSaves results into an SQLite database\n\n    :Elements:\tGenerates mutation-testing-elements compatible JSON file\n\n    :Patches:\tGenerates patch file for each mutation\n\n    :GithubAnnotations:\tPrint GithubAnnotations for mutants\n",
            ..opt(Some("reporters"), None, "Choose reporters:", SharedCli, "Reporting")
        },
        opt(Some("ide-reporter-show-killed"), None, "Makes IDEReporter to also report killed mutations (disabled by default)", SharedCli, "Reporting"),

        // ── Mode / behavior ──
        opt(Some("debug"), Some("debugEnabled"), "Enables Debug Mode: more logs are printed", SharedCli, "Behavior"),
        opt(Some("strict"), None, "Enables Strict Mode: all warning messages are treated as fatal errors", SharedCli, "Behavior"),
        opt(Some("allow-surviving"), None, "Do not treat mutants surviving as an error", SharedCli, "Behavior"),
        opt(Some("mutation-score-threshold"), None, "If mutation score falls under this threshold, and allow-surviving is not enabled, an error result code is returned", SharedCli, "Behavior"),

        // ── Output capture ──
        opt(Some("no-test-output"), None, "Does not capture output from test runs", SharedCli, "Output"),
        opt(Some("no-mutant-output"), None, "Does not capture output from mutant runs", SharedCli, "Output"),
        opt(Some("no-output"), None, "Combines -no-test-output and -no-mutant-output", SharedCli, "Output"),

        // ── Runner-only ──
        OptionMeta { value_hint: "directory", ..opt(Some("ld-search-path"), None, "Library search path", RunnerCli, "Runner") },
        OptionMeta { value_hint: "string", ..opt(Some("coverage-info"), None, "Path to the coverage info file (LLVM's profdata)", RunnerCli, "Runner") },
        opt(Some("debug-coverage"), Some("debug.coverage"), "Print coverage ranges", RunnerCli, "Runner"),
        opt(Some("include-not-covered"), Some("includeNotCovered"), "Include (but do not run) not covered mutants. Disabled by default", RunnerCli, "Runner"),
        opt(Some("dry-run"), Some("dryRunEnabled"), "Skips mutant execution and generation. Disabled by default", RunnerCli, "Runner"),

        // ── YAML-only: Output ──
        opt(None, Some("quiet"), "Suppress informational messages", YamlOnly, "Output"),
        opt(None, Some("silent"), "Suppress all output except errors", YamlOnly, "Output"),
        opt(None, Some("captureTestOutput"), "Capture stdout/stderr from the original test run", YamlOnly, "Output"),
        opt(None, Some("captureMutantOutput"), "Capture stdout/stderr from mutant test runs", YamlOnly, "Output"),

        // ── YAML-only: Mutators ──
        opt(None, Some("mutators"), "Mutator IDs or groups to enable", YamlOnly, "Mutators"),
        opt(None, Some("ignoreMutators"), "Mutator IDs or groups to exclude", YamlOnly, "Mutators"),

        // ── YAML-only: Junk Detection ──
        opt(None, Some("compilationDatabasePath"), "Path to compilation database (compile_commands.json)", YamlOnly, "Junk Detection"),
        opt(None, Some("compilerFlags"), "Additional compiler flags for junk detection", YamlOnly, "Junk Detection"),
        opt(None, Some("junkDetectionDisabled"), "Disable junk detection (AST-based mutant validation)", YamlOnly, "Junk Detection"),

        // ── YAML-only: Filtering ──
        opt(None, Some("includePaths"), "File path regex patterns to include in mutation testing", YamlOnly, "Filtering"),
        opt(None, Some("excludePaths"), "File path regex patterns to exclude from mutation testing", YamlOnly, "Filtering"),
        opt(None, Some("gitDiffRef"), "Git ref to diff against for incremental mutation testing", YamlOnly, "Filtering"),
        opt(None, Some("gitProjectRoot"), "Root directory of the git project", YamlOnly, "Filtering"),

        // ── YAML-only: Parallelization ──
        opt(None, Some("parallelization.workers"), "Number of worker threads for test discovery", YamlOnly, "Parallelization"),
        opt(None, Some("parallelization.executionWorkers"), "Number of worker threads for mutant execution", YamlOnly, "Parallelization"),

        // ── YAML-only: Debug ──
        opt(None, Some("debug.printIR"), "Print LLVM IR", YamlOnly, "Debug"),
        opt(None, Some("debug.printIRBefore"), "Print LLVM IR before mutation", YamlOnly, "Debug"),
        opt(None, Some("debug.printIRAfter"), "Print LLVM IR after mutation", YamlOnly, "Debug"),
        opt(None, Some("debug.printIRToFile"), "Print LLVM IR to a file", YamlOnly, "Debug"),
        opt(None, Some("debug.traceMutants"), "Trace mutant discovery and execution", YamlOnly, "Debug"),
        opt(None, Some("debug.coverage"), "Enable coverage debug output", YamlOnly, "Debug"),
        opt(None, Some("debug.gitDiff"), "Enable git diff debug output", YamlOnly, "Debug"),
        opt(None, Some("debug.filters"), "Enable filter debug output", YamlOnly, "Debug"),
        opt(None, Some("debug.slowIrVerification"), "Enable slow IR verification", YamlOnly, "Debug"),
    ]
}
