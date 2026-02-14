use mull_core::diagnostics::MullDiagnostics as MullDiagnosticsInner;
use mull_filters::FilterMutantsConfig as FilterMutantsConfigInner;

#[cxx::bridge]
mod ffi {
    struct FilterMutantsConfig {
        git_diff_ref: String,
        git_project_root: String,
        debug_git_diff: bool,
        workers: usize,
    }

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

        fn filter_mutants(
            diag: &MullDiagnostics,
            mutant_ids: Vec<String>,
            config: FilterMutantsConfig,
        ) -> Vec<String>;
    }
}

// Re-export FFI types
pub use ffi::CliConfig;
pub use ffi::DebugOptions;
pub use ffi::MullConfig;

// Newtype wrapper for MullDiagnostics to satisfy CXX's orphan rules
pub struct MullDiagnostics(MullDiagnosticsInner);

impl MullDiagnostics {
    pub fn new() -> Self {
        Self(MullDiagnosticsInner::new())
    }

    fn from_inner(inner: MullDiagnosticsInner) -> Self {
        Self(inner)
    }

    fn enable_debug_mode(&self) {
        self.0.enable_debug_mode();
    }

    fn info(&self, message: &str) {
        self.0.info(message);
    }

    fn warning(&self, message: &str) {
        self.0.warning(message);
    }

    fn error(&self, message: &str) -> ! {
        self.0.error(message)
    }

    fn debug(&self, message: &str) {
        self.0.debug(message);
    }

    fn progress(&self, message: &str) {
        self.0.progress(message);
    }
}

impl Default for MullDiagnostics {
    fn default() -> Self {
        Self::new()
    }
}

// Conversion from mull_core types to FFI types

fn convert_debug_options(src: &mull_core::DebugOptions) -> ffi::DebugOptions {
    ffi::DebugOptions {
        print_ir: src.print_ir,
        print_ir_before: src.print_ir_before,
        print_ir_after: src.print_ir_after,
        print_ir_to_file: src.print_ir_to_file,
        trace_mutants: src.trace_mutants,
        coverage: src.coverage,
        git_diff: src.git_diff,
        filters: src.filters,
        slow_ir_verification: src.slow_ir_verification,
    }
}

fn convert_config(src: &mull_core::MullConfig) -> ffi::MullConfig {
    ffi::MullConfig {
        timeout: src.timeout,
        workers: src.workers,
        execution_workers: src.execution_workers,
        debug_enabled: src.debug_enabled,
        quiet: src.quiet,
        silent: src.silent,
        strict: src.strict,
        dry_run_enabled: src.dry_run_enabled,
        capture_test_output: src.capture_test_output,
        capture_mutant_output: src.capture_mutant_output,
        include_not_covered: src.include_not_covered,
        junk_detection_disabled: src.junk_detection_disabled,
        compilation_database_path: src.compilation_database_path.clone(),
        git_diff_ref: src.git_diff_ref.clone(),
        git_project_root: src.git_project_root.clone(),
        mutators: src.mutators.clone(),
        ignore_mutators: src.ignore_mutators.clone(),
        compiler_flags: src.compiler_flags.clone(),
        include_paths: src.include_paths.clone(),
        exclude_paths: src.exclude_paths.clone(),
        debug: convert_debug_options(&src.debug),
        config_path: src.config_path.clone(),
    }
}

fn convert_cli_config(src: &mull_core::CliConfig) -> ffi::CliConfig {
    ffi::CliConfig {
        input_file: src.input_file.clone(),
        test_program: src.test_program.clone(),
        sqlite_report: src.sqlite_report.clone(),
        runner_args: src.runner_args.clone(),
        reporters: src.reporters.clone(),
        report_name: src.report_name.clone(),
        report_dir: src.report_dir.clone(),
        report_patch_base: src.report_patch_base.clone(),
        ide_reporter_show_killed: src.ide_reporter_show_killed,
        strict: src.strict,
        allow_surviving: src.allow_surviving,
        mutation_score_threshold: src.mutation_score_threshold,
        ld_search_paths: src.ld_search_paths.clone(),
        coverage_info: src.coverage_info.clone(),
    }
}

/// FFI wrapper for core initialization
pub struct MullCore {
    diag: MullDiagnostics,
    config: ffi::MullConfig,
}

impl MullCore {
    fn config(&self) -> &ffi::MullConfig {
        &self.config
    }

    fn diag(&self) -> &MullDiagnostics {
        &self.diag
    }
}

/// FFI wrapper for CLI initialization
pub struct MullCLI {
    diag: MullDiagnostics,
    config: ffi::MullConfig,
    cli: ffi::CliConfig,
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

fn init_core_ffi() -> Box<MullCore> {
    let (diag, config) = mull_core::init_core();
    Box::new(MullCore {
        diag: MullDiagnostics::from_inner(diag),
        config: convert_config(&config),
    })
}

fn init_runner_cli(args: Vec<String>, llvm_version: String) -> Box<MullCLI> {
    let inner = mull_core::init_runner_cli(args, llvm_version);
    let (diag, config, cli) = inner.into_parts();
    Box::new(MullCLI {
        diag: MullDiagnostics::from_inner(diag),
        config: convert_config(&config),
        cli: convert_cli_config(&cli),
    })
}

fn init_reporter_cli(args: Vec<String>, llvm_version: String) -> Box<MullCLI> {
    let inner = mull_core::init_reporter_cli(args, llvm_version);
    let (diag, config, cli) = inner.into_parts();
    Box::new(MullCLI {
        diag: MullDiagnostics::from_inner(diag),
        config: convert_config(&config),
        cli: convert_cli_config(&cli),
    })
}

fn filter_mutants(
    diag: &MullDiagnostics,
    mutant_ids: Vec<String>,
    config: ffi::FilterMutantsConfig,
) -> Vec<String> {
    let inner_config = FilterMutantsConfigInner {
        git_diff_ref: config.git_diff_ref,
        git_project_root: config.git_project_root,
        debug_git_diff: config.debug_git_diff,
        workers: config.workers,
    };
    mull_filters::filter_mutants(&diag.0, mutant_ids, inner_config)
}
