mod coverage_filter;
mod file_ranges;
mod filepath_filter;
mod filter_chain;
mod git_diff_filter;
mod manual_filter;
mod mutation_ranges_filter;
mod mutator_groups;

pub use coverage_filter::{has_coverage, CoverageFilter};
pub use filepath_filter::FilePathFilter;
pub use filter_chain::{FilterChain, FilterChainConfig, FilterDecision, FilterResult};
pub use git_diff_filter::GitDiffFilter;
pub use manual_filter::ManualFilter;
pub use mutation_ranges_filter::MutationRangesFilter;
pub use mutator_groups::{expand_mutator_groups, get_group_definitions};

use mull_core::diagnostics::MullDiagnostics;
use mull_state::Mutant;
use std::collections::HashMap;
use std::path::Path;

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
    pub git_diff_paths: Vec<String>,
    pub debug_git_diff: bool,
    pub mutation_ranges: String,
    pub workers: usize,
    pub enable_manual_filter: bool,
    pub include_paths: Vec<String>,
    pub exclude_paths: Vec<String>,
    pub debug_filepath: bool,
}

/// Find the entry sharing the longest path suffix with `path`. Handles cases where the entries
/// have different path prefixes (e.g., sandbox paths) than debug info. Ambiguous matches are `None`.
pub(crate) fn find_by_path_suffix<'a, T>(
    entries: &'a HashMap<String, T>,
    path: &str,
) -> Option<&'a T> {
    let shared_components = |candidate: &str| {
        Path::new(path)
            .components()
            .rev()
            .zip(Path::new(candidate).components().rev())
            .take_while(|(lhs, rhs)| lhs == rhs)
            .count()
    };

    let mut best = None;
    let mut best_length = 0;
    for (candidate, value) in entries {
        let length = shared_components(candidate);
        if length > best_length {
            best = Some(value);
            best_length = length;
        } else if length == best_length {
            best = None;
        }
    }
    best
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
    // If no filters are enabled, return all mutants
    if config.git_diff_ref.is_empty()
        && config.mutation_ranges.is_empty()
        && !config.enable_manual_filter
        && config.include_paths.is_empty()
        && config.exclude_paths.is_empty()
    {
        return mutant_ids;
    }

    // Construct Mutants from identifiers (replacement is not used by GitDiffFilter)
    let mutants: Vec<Mutant> = mutant_ids
        .into_iter()
        .map(|id| Mutant::from_identifier(diag, id, String::new()))
        .collect();

    // Build filter chain
    let filter_config = FilterChainConfig {
        coverage_profile_path: None,
        object_files: &[],
        include_not_covered: false,
        debug_coverage: false,
        git_diff_ref: &config.git_diff_ref,
        git_project_root: &config.git_project_root,
        git_diff_paths: &config.git_diff_paths,
        debug_git_diff: config.debug_git_diff,
        mutation_ranges: &config.mutation_ranges,
        enable_manual_filter: config.enable_manual_filter,
        include_paths: &config.include_paths,
        exclude_paths: &config.exclude_paths,
        debug_filepath: config.debug_filepath,
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

#[cfg(test)]
mod tests {
    use super::find_by_path_suffix;
    use std::collections::HashMap;

    fn entries(paths: &[&str]) -> HashMap<String, String> {
        paths
            .iter()
            .map(|p| (p.to_string(), p.to_string()))
            .collect()
    }

    fn find(paths: &[&str], path: &str) -> Option<String> {
        find_by_path_suffix(&entries(paths), path).cloned()
    }

    #[test]
    fn matches_whole_components_only() {
        assert_eq!(find(&["/build/xutil.cpp"], "/src/util.cpp"), None);
        assert_eq!(
            find(&["/build/a/util.cpp"], "/src/ba/util.cpp"),
            Some("/build/a/util.cpp".into())
        );
    }

    #[test]
    fn nothing_in_common_is_no_match() {
        assert_eq!(find(&["/build/main.cpp"], "/src/util.cpp"), None);
        assert_eq!(find(&[], "/src/util.cpp"), None);
    }

    #[test]
    fn longest_suffix_wins() {
        let paths = [
            "/build/util.cpp",
            "/build/a/util.cpp",
            "/build/x/a/util.cpp",
        ];
        assert_eq!(
            find(&paths, "/src/x/a/util.cpp"),
            Some("/build/x/a/util.cpp".into())
        );
        assert_eq!(find(&paths, "/src/y/a/util.cpp"), None);
    }

    #[test]
    fn tie_is_no_match() {
        let paths = ["/build/a/util.cpp", "/build/b/util.cpp"];
        assert_eq!(find(&paths, "/src/util.cpp"), None);
        assert_eq!(find(&paths, "/src/c/util.cpp"), None);
    }

    #[test]
    fn relative_entry_matches_absolute_path() {
        assert_eq!(find(&["./main.c"], "/src/main.c"), Some("./main.c".into()));
        assert_eq!(find(&["main.c"], "/src/main.c"), Some("main.c".into()));
    }
}
