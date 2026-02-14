mod coverage_filter;
mod filter_chain;
mod git_diff_filter;

pub use coverage_filter::{has_coverage, CoverageFilter};
pub use filter_chain::{FilterChain, FilterChainConfig, FilterDecision, FilterResult};
pub use git_diff_filter::GitDiffFilter;

use mull_core::diagnostics::MullDiagnostics;
use mull_state::Mutant;

/// Filter trait for deciding whether to skip a mutant
pub trait MutantFilter: Send + Sync {
    /// Returns the filter decision for a mutant
    ///
    /// The `diag` and `debug` parameters allow filters to emit debug messages.
    fn filter_decision(
        &self,
        mutant: &Mutant,
        diag: &MullDiagnostics,
        debug: bool,
    ) -> FilterDecision;

    /// Returns the name of this filter
    fn name(&self) -> &str;
}

/// Configuration for the filter_mutants function
pub struct FilterMutantsConfig {
    pub git_diff_ref: String,
    pub git_project_root: String,
    pub debug_git_diff: bool,
    pub workers: usize,
}

/// Filter mutants using the Rust filter chain
///
/// Takes a list of mutant identifiers and returns the IDs of mutants to keep.
/// Currently only applies the GitDiffFilter.
pub fn filter_mutants(
    diag: &MullDiagnostics,
    mutant_ids: Vec<String>,
    config: FilterMutantsConfig,
) -> Vec<String> {
    // If git diff filtering is not enabled, return all mutants
    if config.git_diff_ref.is_empty() {
        return mutant_ids;
    }

    // Construct Mutants from identifiers (replacement is not used by GitDiffFilter)
    let mutants: Vec<Mutant> = mutant_ids
        .into_iter()
        .map(|id| Mutant::from_identifier(diag, id, String::new()))
        .collect();

    // Build filter chain with only GitDiffFilter
    let filter_config = FilterChainConfig {
        coverage_profile_path: None,
        object_files: &[],
        include_not_covered: false,
        debug_coverage: false,
        git_diff_ref: &config.git_diff_ref,
        git_project_root: &config.git_project_root,
        debug_git_diff: config.debug_git_diff,
    };

    let filter_chain = FilterChain::from_config(diag, filter_config);

    if filter_chain.is_empty() {
        // No filters configured, return all
        return mutants.into_iter().map(|m| m.identifier).collect();
    }

    // Run the filter chain
    let result = filter_chain.filter(mutants, diag, config.debug_git_diff, config.workers);

    // Return identifiers of mutants to execute
    result
        .to_execute
        .into_iter()
        .map(|m| m.identifier)
        .collect()
}
