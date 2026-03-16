use crate::{CoverageFilter, FilePathFilter, GitDiffFilter, ManualFilter, MutantFilter};
use mull_core::diag_warning;
use mull_core::diagnostics::MullDiagnostics;
use mull_state::{ExecutionStatus, Mutant};
use mull_tasks::run_parallel_task;

/// Result of filtering mutants
pub struct FilterResult {
    /// Mutants that should be executed
    pub to_execute: Vec<Mutant>,
    /// Mutants that should be reported with a specific status (not executed)
    pub to_report: Vec<(Mutant, ExecutionStatus)>,
}

/// Configuration for building a filter chain
pub struct FilterChainConfig<'a> {
    /// Coverage profile path (if available)
    pub coverage_profile_path: Option<&'a str>,
    /// Object files containing coverage data
    pub object_files: &'a [String],
    /// Include not-covered mutants in report
    pub include_not_covered: bool,
    /// Enable coverage debug output
    pub debug_coverage: bool,
    /// Git ref to diff against (empty = disabled)
    pub git_diff_ref: &'a str,
    /// Git project root directory (empty = current dir)
    pub git_project_root: &'a str,
    /// Enable git diff debug output
    pub debug_git_diff: bool,
    /// Enable manual filter (mull-off/mull-on/mull-ignore comments)
    pub enable_manual_filter: bool,
    /// Include path patterns (regex)
    pub include_paths: &'a [String],
    /// Exclude path patterns (regex)
    pub exclude_paths: &'a [String],
    /// Enable filepath filter debug output
    pub debug_filepath: bool,
}

/// A chain of filters that can be applied to mutants
pub struct FilterChain {
    filters: Vec<Box<dyn MutantFilter>>,
}

impl Default for FilterChain {
    fn default() -> Self {
        Self::new()
    }
}

impl FilterChain {
    /// Create a new empty filter chain
    pub fn new() -> Self {
        Self {
            filters: Vec::new(),
        }
    }

    /// Create a filter chain from configuration
    ///
    /// Automatically adds filters based on config settings:
    /// - CoverageFilter: if coverage_profile_path is provided or include_not_covered is set
    /// - GitDiffFilter: if git_diff_ref is not empty
    pub fn from_config(diag: &MullDiagnostics, config: FilterChainConfig) -> Self {
        let mut chain = Self::new();

        // Add coverage filter if coverage data is available or include_not_covered is set
        if config.coverage_profile_path.is_some() || config.include_not_covered {
            let coverage_filter = if let Some(profile_path) = config.coverage_profile_path {
                CoverageFilter::from_profile_path(
                    diag,
                    profile_path,
                    config.object_files,
                    config.include_not_covered,
                    config.debug_coverage,
                )
            } else {
                CoverageFilter::empty(config.include_not_covered)
            };
            chain.add(coverage_filter);
        }

        // Add git diff filter if configured
        if !config.git_diff_ref.is_empty() {
            if config.git_project_root.is_empty() {
                diag_warning!(
                    diag,
                    "gitDiffRef option has been provided but the path to the Git project root has not been specified via gitProjectRoot. The incremental testing will be disabled."
                );
            } else if !std::path::Path::new(config.git_project_root).is_dir() {
                diag_warning!(
                    diag,
                    "directory provided by gitProjectRoot does not exist, the incremental testing will be disabled: {}",
                    config.git_project_root
                );
            } else {
                let git_diff_filter = GitDiffFilter::from_git_diff(
                    diag,
                    config.git_project_root,
                    config.git_diff_ref,
                    config.debug_git_diff,
                );
                chain.add(git_diff_filter);
            }
        }

        // Add file path filter if patterns are configured
        if !config.include_paths.is_empty() || !config.exclude_paths.is_empty() {
            let filepath_filter = FilePathFilter::new(
                diag,
                config.include_paths,
                config.exclude_paths,
                config.debug_filepath,
            );
            chain.add(filepath_filter);
        }

        // Add manual filter if enabled
        if config.enable_manual_filter {
            chain.add(ManualFilter::new());
        }

        chain
    }

    /// Add a filter to the chain
    pub fn add<F: MutantFilter + 'static>(&mut self, filter: F) {
        self.filters.push(Box::new(filter));
    }

    /// Check if any filters are registered
    pub fn is_empty(&self) -> bool {
        self.filters.is_empty()
    }

    /// Apply all filters to a list of mutants
    ///
    /// Each filter runs as a separate parallel task with its own progress indicator.
    /// Returns mutants split into:
    /// - `to_execute`: mutants that passed all filters
    /// - `to_report`: mutants that should be reported with a specific status
    pub fn filter(
        &self,
        mutants: Vec<Mutant>,
        diag: &MullDiagnostics,
        debug: bool,
        workers: usize,
    ) -> FilterResult {
        let mut current_mutants = mutants;
        let mut to_report: Vec<(Mutant, ExecutionStatus)> = Vec::new();

        for filter in &self.filters {
            if current_mutants.is_empty() {
                break;
            }

            let filter_name = filter.name().to_string();
            let task_name = format!("Applying {} filter", filter_name);

            // Process mutants in parallel for this filter
            let results: Vec<(Mutant, FilterDecision)> = run_parallel_task(
                diag,
                &task_name,
                workers,
                current_mutants,
                |_idx, mutant| {
                    let decision = filter.filter_decision(&mutant, diag, debug);
                    (mutant, decision)
                },
            );

            // Separate results
            let mut kept = Vec::new();
            for (mutant, decision) in results {
                match decision {
                    FilterDecision::Keep => {
                        kept.push(mutant);
                    }
                    FilterDecision::ReportAs(status) => {
                        to_report.push((mutant, status));
                    }
                    FilterDecision::Skip => {
                        // Debug message already logged by filter if needed
                    }
                }
            }

            current_mutants = kept;
        }

        FilterResult {
            to_execute: current_mutants,
            to_report,
        }
    }
}

/// Decision made by a filter for a single mutant
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FilterDecision {
    /// Keep the mutant (pass to next filter or execute)
    Keep,
    /// Don't execute but report with this status
    ReportAs(ExecutionStatus),
    /// Skip entirely (don't execute, don't report)
    Skip,
}
