use mull_core::diagnostics::MullDiagnostics as MullDiagnosticsInner;
use mull_filters::FilterMutantsConfig as FilterMutantsConfigInner;

#[cxx::bridge]
mod ffi {
    struct FilterMutantsConfig {
        git_diff_ref: String,
        git_project_root: String,
        debug_git_diff: bool,
        workers: usize,
        enable_manual_filter: bool,
        include_paths: Vec<String>,
        exclude_paths: Vec<String>,
        debug_filepath: bool,
    }

    struct MutatorGroupDef {
        name: String,
        description: String,
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

        fn filter_mutants(
            diag: &MullDiagnostics,
            mutant_ids: Vec<String>,
            config: FilterMutantsConfig,
        ) -> Vec<String>;

        fn get_clang_resource_dir(compiler_path: &str) -> String;

        fn expand_mutator_groups(groups: Vec<String>) -> Vec<String>;

        fn get_mutator_group_definitions() -> Vec<MutatorGroupDef>;
    }
}

// Re-export FFI types
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

fn init_core_ffi() -> Box<MullCore> {
    let (diag, config) = mull_core::init_core();
    Box::new(MullCore {
        diag: MullDiagnostics::from_inner(diag),
        config: convert_config(&config),
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
        enable_manual_filter: config.enable_manual_filter,
        include_paths: config.include_paths,
        exclude_paths: config.exclude_paths,
        debug_filepath: config.debug_filepath,
    };
    mull_filters::filter_mutants(&diag.0, mutant_ids, inner_config)
}

fn get_clang_resource_dir(compiler_path: &str) -> String {
    use std::process::Command;

    let output = Command::new(compiler_path)
        .arg("-print-resource-dir")
        .output();

    match output {
        Ok(out) if out.status.success() => String::from_utf8_lossy(&out.stdout).trim().to_string(),
        _ => String::new(),
    }
}

fn expand_mutator_groups(groups: Vec<String>) -> Vec<String> {
    let group_refs: Vec<&str> = groups.iter().map(|s| s.as_str()).collect();
    let expanded = mull_filters::expand_mutator_groups(&group_refs);
    let mut result: Vec<String> = expanded.into_iter().collect();
    result.sort();
    result
}

fn get_mutator_group_definitions() -> Vec<ffi::MutatorGroupDef> {
    mull_filters::get_group_definitions()
        .into_iter()
        .map(|(name, description)| ffi::MutatorGroupDef { name, description })
        .collect()
}
