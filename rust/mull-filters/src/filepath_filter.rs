use crate::{FilterDecision, MutantFilter};
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_debug, diag_warning};
use mull_state::Mutant;
use regex::Regex;
use std::collections::HashMap;
use std::sync::RwLock;

/// Filter that includes/excludes mutants based on source file paths.
///
/// Logic:
/// - If include patterns exist, path must match at least one
/// - If path matches any exclude pattern, it's excluded
/// - Results are cached per file path
pub struct FilePathFilter {
    include_patterns: Vec<Regex>,
    exclude_patterns: Vec<Regex>,
    cache: RwLock<HashMap<String, bool>>,
    debug: bool,
}

impl FilePathFilter {
    /// Compile pattern strings into Regex objects.
    /// Invalid patterns are logged as warnings and skipped.
    fn compile_patterns(
        diag: &MullDiagnostics,
        patterns: &[String],
        debug_verb: &str,
        error_kind: &str,
        debug: bool,
    ) -> Vec<Regex> {
        patterns
            .iter()
            .filter_map(|pattern| {
                if debug {
                    diag_debug!(diag, "FilePath: {}: {}", debug_verb, pattern);
                }
                match Regex::new(pattern) {
                    Ok(re) => Some(re),
                    Err(e) => {
                        diag_warning!(
                            diag,
                            "Invalid regex for {}: '{}' has been ignored. Error: {}",
                            error_kind,
                            pattern,
                            e
                        );
                        None
                    }
                }
            })
            .collect()
    }

    /// Create a new FilePathFilter from include/exclude pattern strings.
    ///
    /// Invalid regex patterns are logged as warnings and skipped.
    pub fn new(
        diag: &MullDiagnostics,
        include_paths: &[String],
        exclude_paths: &[String],
        debug: bool,
    ) -> Self {
        Self {
            include_patterns: Self::compile_patterns(
                diag,
                include_paths,
                "including",
                "include-path",
                debug,
            ),
            exclude_patterns: Self::compile_patterns(
                diag,
                exclude_paths,
                "excluding",
                "exclude-path",
                debug,
            ),
            cache: RwLock::new(HashMap::new()),
            debug,
        }
    }

    /// Check if a path is allowed (not skipped).
    fn is_allowed(&self, filepath: &str) -> bool {
        // Check cache first (read lock)
        {
            let cache = self.cache.read().unwrap();
            if let Some(&allowed) = cache.get(filepath) {
                return allowed;
            }
        }

        // Compute and cache
        let allowed = self.compute_allowed(filepath);
        self.cache
            .write()
            .unwrap()
            .insert(filepath.to_string(), allowed);
        allowed
    }

    /// Compute whether a path is allowed without caching.
    fn compute_allowed(&self, filepath: &str) -> bool {
        // If include patterns exist, path must match at least one
        let mut allowed = if self.include_patterns.is_empty() {
            true
        } else {
            self.include_patterns.iter().any(|re| re.is_match(filepath))
        };

        // If allowed so far, check exclude patterns
        if allowed {
            allowed = !self.exclude_patterns.iter().any(|re| re.is_match(filepath));
        }

        allowed
    }

    /// Check if the filter has any patterns configured.
    pub fn is_empty(&self) -> bool {
        self.include_patterns.is_empty() && self.exclude_patterns.is_empty()
    }
}

impl MutantFilter for FilePathFilter {
    fn filter_decision(
        &self,
        mutant: &Mutant,
        diag: &MullDiagnostics,
        _debug: bool,
    ) -> FilterDecision {
        let filepath = &mutant.location.filepath;

        if filepath.is_empty() {
            return FilterDecision::Skip;
        }

        if self.is_allowed(filepath) {
            FilterDecision::Keep
        } else {
            if self.debug {
                diag_debug!(
                    diag,
                    "FilePathFilter: skipping mutant {}:{}:{} - path excluded",
                    filepath,
                    mutant.location.begin_line,
                    mutant.location.begin_column
                );
            }
            FilterDecision::Skip
        }
    }

    fn name(&self) -> &str {
        "file path"
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn mock_diag() -> MullDiagnostics {
        MullDiagnostics::new()
    }

    #[test]
    fn test_no_patterns_allows_all() {
        let filter = FilePathFilter::new(&mock_diag(), &[], &[], false);
        assert!(filter.is_allowed("/any/path/file.cpp"));
    }

    #[test]
    fn test_include_pattern_filters() {
        let filter = FilePathFilter::new(&mock_diag(), &["src/.*".to_string()], &[], false);
        assert!(filter.is_allowed("src/main.cpp"));
        assert!(!filter.is_allowed("test/main.cpp"));
    }

    #[test]
    fn test_exclude_pattern_filters() {
        let filter = FilePathFilter::new(&mock_diag(), &[], &["test/.*".to_string()], false);
        assert!(filter.is_allowed("src/main.cpp"));
        assert!(!filter.is_allowed("test/main.cpp"));
    }

    #[test]
    fn test_include_and_exclude() {
        let filter = FilePathFilter::new(
            &mock_diag(),
            &["src/.*".to_string()],
            &["src/generated/.*".to_string()],
            false,
        );
        assert!(filter.is_allowed("src/main.cpp"));
        assert!(!filter.is_allowed("src/generated/code.cpp"));
        assert!(!filter.is_allowed("test/main.cpp"));
    }

    #[test]
    fn test_invalid_regex_ignored() {
        let filter = FilePathFilter::new(
            &mock_diag(),
            &["[invalid".to_string(), "valid.*".to_string()],
            &[],
            false,
        );
        // Should still have one valid pattern
        assert!(!filter.is_empty());
        assert!(filter.is_allowed("valid_file.cpp"));
    }

    #[test]
    fn test_caching() {
        let filter = FilePathFilter::new(&mock_diag(), &["src/.*".to_string()], &[], false);

        // First call computes
        assert!(filter.is_allowed("src/main.cpp"));

        // Second call uses cache
        assert!(filter.is_allowed("src/main.cpp"));

        // Verify cache has entry
        let cache = filter.cache.read().unwrap();
        assert!(cache.contains_key("src/main.cpp"));
    }
}
