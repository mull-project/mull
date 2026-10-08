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

    let mut object_paths = vec![];
    // Ignore objects without coverage info.
    for object in object_files {
        if has_coverage(object) {
            object_paths.push(PathBuf::from(object));
        } else if debug_coverage {
            diag_warning!(diag, "mull-coverage: no coverage mapping in '{}'", object);
        }
    }

    // Load coverage mapping from object files
    let coverage_mapping = match CoverageMapping::new(&object_paths, &profile, true) {
        Ok(cm) => cm,
        Err(e) => {
            diag_warning!(diag, "Cannot parse coverage mapping: {}", e);
            return uncovered_ranges;
        }
    };

    // Generate the coverage report
    let report = match coverage_mapping.generate_report() {
        Ok(r) => r,
        Err(e) => {
            diag_warning!(diag, "Cannot generate coverage report: {}", e);
            return uncovered_ranges;
        }
    };

    // A profile in which nothing at all was executed (a mismatch between the profile and the
    // objects, or tests that did not run) would mark every mutant as not covered.
    let any_hit = report
        .files
        .values()
        .any(|file| file.hits.values().any(|&count| count > 0));
    if !any_hit {
        diag_warning!(
            diag,
            "The coverage report shows no executed code, coverage disabling filtering"
        );
        return uncovered_ranges;
    }

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

/// Check if an object file has coverage instrumentation, i.e. both the coverage map and the
/// coverage functions sections that llvm_profparser needs in order to read it.
pub fn has_coverage(object_path: &str) -> bool {
    use object::read::ReadCache;
    use object::{Object, ObjectSection};

    let Ok(file) = std::fs::File::open(object_path) else {
        return false;
    };

    // Reads only the headers instead of the whole file, which may be gigabytes of debug info.
    let cache = ReadCache::new(file);
    let Ok(object) = object::File::parse(&cache) else {
        return false;
    };

    // Section names differ per platform (__llvm_covmap on macOS and Linux, .lcovmap on Windows)
    let mut has_map = false;
    let mut has_functions = false;
    for section in object.sections() {
        if let Ok(name) = section.name() {
            has_map |= name.ends_with("llvm_covmap");
            has_functions |= name.ends_with("llvm_covfun");
        }
    }
    has_map && has_functions
}

#[cfg(test)]
mod tests {
    use super::*;
    /// A minimal relocatable ELF64 file with empty sections of the given names.
    fn elf_with_sections(names: &[&str]) -> Vec<u8> {
        let mut strtab = vec![0u8];
        let mut name_offsets = vec![];
        for name in names.iter().chain([&".shstrtab"]) {
            name_offsets.push(strtab.len() as u32);
            strtab.extend_from_slice(name.as_bytes());
            strtab.push(0);
        }
        let section_count = names.len() as u16 + 2; // null section + the named ones + .shstrtab
        let strtab_offset = 64 + 64 * section_count as u64;

        let mut elf = vec![0x7f, b'E', b'L', b'F', 2, 1, 1, 0];
        elf.resize(16, 0);
        elf.extend_from_slice(&1u16.to_le_bytes()); // e_type: ET_REL
        elf.extend_from_slice(&183u16.to_le_bytes()); // e_machine: EM_AARCH64
        elf.extend_from_slice(&1u32.to_le_bytes()); // e_version
        elf.extend_from_slice(&0u64.to_le_bytes()); // e_entry
        elf.extend_from_slice(&0u64.to_le_bytes()); // e_phoff
        elf.extend_from_slice(&64u64.to_le_bytes()); // e_shoff
        elf.extend_from_slice(&0u32.to_le_bytes()); // e_flags
        elf.extend_from_slice(&64u16.to_le_bytes()); // e_ehsize
        elf.extend_from_slice(&0u16.to_le_bytes()); // e_phentsize
        elf.extend_from_slice(&0u16.to_le_bytes()); // e_phnum
        elf.extend_from_slice(&64u16.to_le_bytes()); // e_shentsize
        elf.extend_from_slice(&section_count.to_le_bytes()); // e_shnum
        elf.extend_from_slice(&(section_count - 1).to_le_bytes()); // e_shstrndx

        elf.extend_from_slice(&[0u8; 64]); // null section
        for (index, name_offset) in name_offsets.iter().enumerate() {
            let is_strtab = index == names.len();
            elf.extend_from_slice(&name_offset.to_le_bytes()); // sh_name
            elf.extend_from_slice(&(if is_strtab { 3u32 } else { 1u32 }).to_le_bytes()); // sh_type
            elf.extend_from_slice(&0u64.to_le_bytes()); // sh_flags
            elf.extend_from_slice(&0u64.to_le_bytes()); // sh_addr
            elf.extend_from_slice(&(if is_strtab { strtab_offset } else { 0 }).to_le_bytes()); // sh_offset
            elf.extend_from_slice(&(if is_strtab { strtab.len() as u64 } else { 0 }).to_le_bytes()); // sh_size
            elf.extend_from_slice(&0u32.to_le_bytes()); // sh_link
            elf.extend_from_slice(&0u32.to_le_bytes()); // sh_info
            elf.extend_from_slice(&1u64.to_le_bytes()); // sh_addralign
            elf.extend_from_slice(&0u64.to_le_bytes()); // sh_entsize
        }
        elf.extend_from_slice(&strtab);
        elf
    }

    fn object_with_sections(file_name: &str, sections: &[&str]) -> String {
        let path = std::env::temp_dir().join(format!(
            "mull-coverage-filter-{}-{}",
            std::process::id(),
            file_name
        ));
        std::fs::write(&path, elf_with_sections(sections)).unwrap();
        path.to_string_lossy().to_string()
    }

    #[test]
    fn has_coverage_needs_map_and_functions() {
        let both = object_with_sections("both", &["__llvm_covmap", "__llvm_covfun"]);
        assert!(has_coverage(&both));
        let _ = std::fs::remove_file(both);
    }

    #[test]
    fn has_coverage_rejects_partial_mapping() {
        let map_only = object_with_sections("map-only", &["__llvm_covmap"]);
        assert!(!has_coverage(&map_only));
        let _ = std::fs::remove_file(map_only);

        let functions_only = object_with_sections("functions-only", &["__llvm_covfun"]);
        assert!(!has_coverage(&functions_only));
        let _ = std::fs::remove_file(functions_only);
    }

    #[test]
    fn has_coverage_rejects_object_without_coverage() {
        let none = object_with_sections("none", &[".text", "__llvm_prf_cnts"]);
        assert!(!has_coverage(&none));
        let _ = std::fs::remove_file(none);
    }

    #[test]
    fn has_coverage_rejects_missing_and_unparsable_files() {
        assert!(!has_coverage("/does/not/exist"));

        let text =
            std::env::temp_dir().join(format!("mull-coverage-filter-{}-text", std::process::id()));
        std::fs::write(&text, "not an object file").unwrap();
        assert!(!has_coverage(&text.to_string_lossy()));
        let _ = std::fs::remove_file(text);
    }
}
