use crate::mutator_groups::expand_mutator_groups;
use crate::{FilterDecision, MutantFilter};
use mull_core::diag_debug;
use mull_core::diagnostics::MullDiagnostics;
use mull_state::Mutant;
use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::path::PathBuf;
use std::sync::RwLock;

/// A range of lines that should be ignored for mutation testing.
#[derive(Debug, Clone)]
struct IgnoreRange {
    /// First line of the range (1-indexed)
    begin: u64,
    /// Last line of the range (1-indexed, inclusive)
    end: u64,
    /// Set of mutator IDs that should be ignored in this range
    ignored_mutators: HashSet<String>,
}

impl IgnoreRange {
    fn contains_line(&self, line: u64) -> bool {
        line >= self.begin && line <= self.end
    }

    fn ignores_mutator(&self, mutator_id: &str) -> bool {
        self.ignored_mutators.contains(mutator_id)
    }
}

/// Filter that respects manual ignore comments in source files.
///
/// Recognizes the following comment patterns:
/// - `mull-off` / `mull-on`: Block-level ignore (all mutants in the range)
/// - `mull-off: <mutators>` / `mull-on`: Block-level ignore for specific mutators
/// - `mull-ignore`: Ignore mutants on the same line
/// - `mull-ignore: <mutators>`: Ignore specific mutators on the same line
/// - `mull-ignore-next`: Ignore mutants on the next line
/// - `mull-ignore-next: <mutators>`: Ignore specific mutators on the next line
pub struct ManualFilter {
    /// Cache of parsed ignore ranges per file
    cache: RwLock<HashMap<PathBuf, Vec<IgnoreRange>>>,
}

impl ManualFilter {
    pub fn new() -> Self {
        Self {
            cache: RwLock::new(HashMap::new()),
        }
    }

    /// Parse mutator specifications after a colon (e.g., "cxx_add_to_sub, cxx_arithmetic")
    fn parse_mutators(line: &str, needle: &str) -> HashSet<String> {
        let Some(pos) = line.find(needle) else {
            return expand_mutator_groups(&["cxx_all"]);
        };

        let after_needle = &line[pos + needle.len()..];
        let cleaned: String = after_needle
            .chars()
            .filter(|c| c.is_alphanumeric() || *c == '_' || *c == ',')
            .collect();

        let mutators: Vec<&str> = cleaned
            .split(',')
            .map(str::trim)
            .filter(|s| !s.is_empty())
            .collect();

        if mutators.is_empty() {
            expand_mutator_groups(&["cxx_all"])
        } else {
            expand_mutator_groups(&mutators)
        }
    }

    /// Scan a file and extract all ignore ranges
    fn scan_file(filepath: &str) -> Vec<IgnoreRange> {
        let Ok(file) = File::open(filepath) else {
            return Vec::new();
        };

        let reader = BufReader::new(file);
        let mut ranges = Vec::new();
        let mut block_state: Option<(u64, HashSet<String>)> = None;

        for (i, line_result) in reader.lines().enumerate() {
            let line_num = (i + 1) as u64;
            let Ok(line) = line_result else {
                continue;
            };

            if let Some((begin, ref ignored_mutators)) = block_state {
                // Inside a mull-off block, look for mull-on
                if line.contains("mull-on") {
                    ranges.push(IgnoreRange {
                        begin,
                        end: line_num,
                        ignored_mutators: ignored_mutators.clone(),
                    });
                    block_state = None;
                }
            } else if line.contains("mull-off") {
                block_state = Some((line_num, Self::parse_mutators(&line, "mull-off:")));
            } else if line.contains("mull-ignore-next") {
                // Must check mull-ignore-next before mull-ignore (substring match)
                ranges.push(IgnoreRange {
                    begin: line_num + 1,
                    end: line_num + 1,
                    ignored_mutators: Self::parse_mutators(&line, "mull-ignore-next:"),
                });
            } else if line.contains("mull-ignore") {
                ranges.push(IgnoreRange {
                    begin: line_num,
                    end: line_num,
                    ignored_mutators: Self::parse_mutators(&line, "mull-ignore:"),
                });
            }
        }

        // If we're still in a block at EOF, close it
        if let Some((begin, ignored_mutators)) = block_state {
            ranges.push(IgnoreRange {
                begin,
                end: u64::MAX,
                ignored_mutators,
            });
        }

        ranges
    }

    /// Check if a mutant should be skipped based on cached ignore ranges
    fn should_skip(&self, filepath: &str, line: u64, mutator_id: &str) -> bool {
        let path = PathBuf::from(filepath);

        // Try read lock first (fast path for cache hit)
        {
            let cache = self.cache.read().unwrap();
            if let Some(ranges) = cache.get(&path) {
                return ranges
                    .iter()
                    .any(|r| r.contains_line(line) && r.ignores_mutator(mutator_id));
            }
        }

        // Cache miss: compute and store
        let ranges = Self::scan_file(filepath);
        let result = ranges
            .iter()
            .any(|r| r.contains_line(line) && r.ignores_mutator(mutator_id));

        // Store in cache
        self.cache.write().unwrap().insert(path, ranges);

        result
    }
}

impl Default for ManualFilter {
    fn default() -> Self {
        Self::new()
    }
}

impl MutantFilter for ManualFilter {
    fn filter_decision(
        &self,
        mutant: &Mutant,
        diag: &MullDiagnostics,
        debug: bool,
    ) -> FilterDecision {
        let filepath = &mutant.location.filepath;
        let line = mutant.location.begin_line;
        let mutator_id = &mutant.mutator;

        // Skip if no source location
        if filepath.is_empty() || line == 0 {
            return FilterDecision::Skip;
        }

        if self.should_skip(filepath, line, mutator_id) {
            if debug {
                diag_debug!(
                    diag,
                    "ManualFilter: skipping mutant {}:{}:{} [{}] - matches ignore range",
                    filepath,
                    line,
                    mutant.location.begin_column,
                    mutator_id
                );
            }
            return FilterDecision::Skip;
        }

        FilterDecision::Keep
    }

    fn name(&self) -> &str {
        "manually ignored mutants"
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_mutators_with_specific() {
        let line = "// mull-ignore: cxx_add_to_sub";
        let result = ManualFilter::parse_mutators(line, "mull-ignore:");
        assert!(result.contains("cxx_add_to_sub"));
        assert_eq!(result.len(), 1);
    }

    #[test]
    fn test_parse_mutators_with_group() {
        let line = "// mull-off: cxx_arithmetic";
        let result = ManualFilter::parse_mutators(line, "mull-off:");
        assert!(result.contains("cxx_add_to_sub"));
        assert!(result.contains("cxx_sub_to_add"));
    }

    #[test]
    fn test_parse_mutators_default() {
        let line = "// mull-ignore";
        let result = ManualFilter::parse_mutators(line, "mull-ignore:");
        // Should default to cxx_all
        assert!(result.contains("cxx_add_to_sub"));
        assert!(result.contains("cxx_eq_to_ne"));
    }

    #[test]
    fn test_parse_mutators_multiple() {
        let line = "// mull-ignore: cxx_add_to_sub, cxx_sub_to_add";
        let result = ManualFilter::parse_mutators(line, "mull-ignore:");
        assert!(result.contains("cxx_add_to_sub"));
        assert!(result.contains("cxx_sub_to_add"));
        assert_eq!(result.len(), 2);
    }

    #[test]
    fn test_ignore_range_contains_line() {
        let range = IgnoreRange {
            begin: 10,
            end: 20,
            ignored_mutators: HashSet::new(),
        };
        assert!(!range.contains_line(9));
        assert!(range.contains_line(10));
        assert!(range.contains_line(15));
        assert!(range.contains_line(20));
        assert!(!range.contains_line(21));
    }
}
