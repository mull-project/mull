use crate::{FilterDecision, MutantFilter};
use llvm_profparser::coverage::coverage_mapping::CoverageMapping;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_debug, diag_warning};
use mull_state::{ExecutionStatus, Mutant};
use std::collections::HashMap;
use std::path::{Path, PathBuf};

/// Represents a source code range that is not covered by tests
#[derive(Debug, Clone)]
struct CoverageRange {
    line_begin: u64,
    column_begin: u64,
    line_end: u64,
    column_end: u64,
}

/// Filter that skips mutants in code not covered by tests
pub struct CoverageFilter {
    /// Map of file path -> uncovered ranges
    uncovered_ranges: HashMap<String, Vec<CoverageRange>>,
    /// Whether to include uncovered mutants anyway
    include_not_covered: bool,
}

impl CoverageFilter {
    /// Create a new coverage filter by loading coverage data from profile file and object files
    pub fn from_profile_path(
        diag: &MullDiagnostics,
        profile_path: &str,
        object_files: &[String],
        include_not_covered: bool,
        debug_coverage: bool,
    ) -> Self {
        let uncovered_ranges = if profile_path.is_empty() {
            HashMap::new()
        } else {
            load_coverage_from_profile_path(diag, profile_path, object_files, debug_coverage)
        };

        Self {
            uncovered_ranges,
            include_not_covered,
        }
    }

    /// Create an empty coverage filter (no filtering)
    pub fn empty(include_not_covered: bool) -> Self {
        Self {
            uncovered_ranges: HashMap::new(),
            include_not_covered,
        }
    }

    /// Check if a mutant is covered by tests
    pub fn covered(&self, mutant: &Mutant) -> bool {
        if self.uncovered_ranges.is_empty() {
            return true;
        }

        let path = &mutant.location.filepath;

        // Try exact path match first
        let ranges = if let Some(r) = self.uncovered_ranges.get(path) {
            r
        } else {
            // Try matching by filename - handles cases where coverage data has
            // different paths (e.g., sandbox paths) than debug info
            let mutant_filename = Path::new(path).file_name();
            let mut found_ranges = None;
            for (cov_path, ranges) in &self.uncovered_ranges {
                let cov_filename = Path::new(cov_path).file_name();
                if mutant_filename == cov_filename {
                    found_ranges = Some(ranges);
                    break;
                }
            }
            match found_ranges {
                Some(r) => r,
                None => return true, // Path not in coverage map means covered
            }
        };

        let line = mutant.location.begin_line;
        let column = mutant.location.begin_column;

        // Check if mutant location falls within any uncovered range
        for range in ranges {
            let in_range_start = (line > range.line_begin)
                || (line == range.line_begin && column >= range.column_begin);
            let in_range_end =
                (line < range.line_end) || (line == range.line_end && column <= range.column_end);

            if in_range_start && in_range_end {
                return false; // Found in uncovered range
            }
        }

        true
    }
}

impl MutantFilter for CoverageFilter {
    fn filter_decision(
        &self,
        mutant: &Mutant,
        _diag: &MullDiagnostics,
        _debug: bool,
    ) -> FilterDecision {
        if self.covered(mutant) {
            FilterDecision::Keep
        } else if self.include_not_covered {
            // Not covered but user wants to see them in report
            FilterDecision::ReportAs(ExecutionStatus::NotCovered)
        } else {
            FilterDecision::Skip
        }
    }

    fn name(&self) -> &str {
        "coverage"
    }
}

/// Load coverage data from a profile path and object files
fn load_coverage_from_profile_path(
    diag: &MullDiagnostics,
    profile_path: &str,
    object_files: &[String],
    debug_coverage: bool,
) -> HashMap<String, Vec<CoverageRange>> {
    let mut uncovered_ranges: HashMap<String, Vec<CoverageRange>> = HashMap::new();

    // Parse the instrumentation profile
    let profile = match llvm_profparser::parse(profile_path) {
        Ok(p) => p,
        Err(e) => {
            diag_warning!(
                diag,
                "Cannot read coverage profile '{}': {}",
                profile_path,
                e
            );
            return uncovered_ranges;
        }
    };

    // Convert object paths to PathBuf
    let object_paths: Vec<PathBuf> = object_files.iter().map(PathBuf::from).collect();

    // Load coverage mapping from object files
    let coverage_mapping = match CoverageMapping::new(&object_paths, &profile, true) {
        Ok(cm) => cm,
        Err(e) => {
            diag_warning!(diag, "Cannot parse coverage mapping: {}", e);
            return uncovered_ranges;
        }
    };

    // Generate the coverage report
    let report = coverage_mapping.generate_report();

    // Extract uncovered regions (those with 0 hits)
    for (file_path, coverage_result) in &report.files {
        let file_str = file_path.to_string_lossy().to_string();

        for (loc, &count) in &coverage_result.hits {
            if count == 0 {
                let range = CoverageRange {
                    line_begin: loc.line_start as u64,
                    column_begin: loc.column_start as u64,
                    line_end: loc.line_end as u64,
                    column_end: loc.column_end as u64,
                };

                if debug_coverage {
                    diag_debug!(
                        diag,
                        "mull-coverage: {}:{}:{}:{}:{}",
                        file_str,
                        range.line_begin,
                        range.column_begin,
                        range.line_end,
                        range.column_end
                    );
                }

                // Store for both relative and absolute path
                uncovered_ranges
                    .entry(file_str.clone())
                    .or_default()
                    .push(range.clone());

                // Also store with absolute path
                if let Ok(abs_path) = Path::new(&file_str).canonicalize() {
                    uncovered_ranges
                        .entry(abs_path.to_string_lossy().to_string())
                        .or_default()
                        .push(range);
                }
            }
        }
    }

    uncovered_ranges
}

/// Check if an executable has coverage instrumentation by looking for llvm_covmap section
pub fn has_coverage(object_path: &str) -> bool {
    use object::{Object, ObjectSection};

    let data = match std::fs::read(object_path) {
        Ok(d) => d,
        Err(_) => return false,
    };

    let file = match object::File::parse(&*data) {
        Ok(f) => f,
        Err(_) => return false,
    };

    for section in file.sections() {
        if let Ok(name) = section.name() {
            // Check for coverage map section (different names on different platforms)
            // macOS: __llvm_covmap (in __DATA segment)
            // Linux: __llvm_covmap or .llvm_covmap
            if name.ends_with("llvm_covmap") {
                return true;
            }
        }
    }

    false
}
