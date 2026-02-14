use crate::{FilterDecision, MutantFilter};
use git2::{DiffOptions, Repository};
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_debug, diag_info, diag_warning};
use mull_state::Mutant;
use std::collections::HashMap;
use std::path::Path;

/// A range of lines in a file that were changed
#[derive(Debug, Clone)]
struct LineRange {
    start: u32,
    count: u32,
}

impl LineRange {
    fn contains(&self, line: u64) -> bool {
        let start = self.start as u64;
        let end = start + self.count as u64 - 1;
        start <= line && line <= end
    }
}

/// Git diff information: map of file path -> changed line ranges
type GitDiffInfo = HashMap<String, Vec<LineRange>>;

/// Filter that skips mutants not in git diff
pub struct GitDiffFilter {
    diff_info: GitDiffInfo,
    debug: bool,
}

impl GitDiffFilter {
    /// Create a new git diff filter by reading diff from git
    pub fn from_git_diff(
        diag: &MullDiagnostics,
        git_project_root: &str,
        git_diff_ref: &str,
        debug: bool,
    ) -> Self {
        // Expand to absolute path like C++ version does
        let abs_project_root = Path::new(git_project_root)
            .canonicalize()
            .map(|p| p.to_string_lossy().to_string())
            .unwrap_or_else(|_| git_project_root.to_string());

        diag_info!(
            diag,
            "Incremental testing using Git Diff is enabled.\n- Git ref: {}\n- Git project root: {}",
            git_diff_ref,
            abs_project_root
        );
        let diff_info = read_git_diff(diag, &abs_project_root, git_diff_ref);
        Self { diff_info, debug }
    }

    /// Create an empty filter (no filtering)
    pub fn empty() -> Self {
        Self {
            diff_info: HashMap::new(),
            debug: false,
        }
    }
}

impl MutantFilter for GitDiffFilter {
    fn filter_decision(
        &self,
        mutant: &Mutant,
        diag: &MullDiagnostics,
        _debug: bool,
    ) -> FilterDecision {
        let file_path = &mutant.location.filepath;
        let line = mutant.location.begin_line;
        let column = mutant.location.begin_column;

        // If no diff info, skip all
        if self.diff_info.is_empty() {
            if self.debug {
                diag_debug!(
                    diag,
                    "GitDiffFilter: git diff is empty. Skipping mutant: {}:{}:{}",
                    file_path,
                    line,
                    column
                );
            }
            return FilterDecision::Skip;
        }

        // Try to find the file in diff info
        let ranges = if let Some(r) = self.diff_info.get(file_path) {
            r
        } else {
            // Try matching by filename only (handles different path prefixes)
            let mutant_filename = Path::new(file_path).file_name();
            let mut found_ranges = None;
            for (diff_path, ranges) in &self.diff_info {
                let diff_filename = Path::new(diff_path).file_name();
                if mutant_filename == diff_filename {
                    found_ranges = Some(ranges);
                    break;
                }
            }
            match found_ranges {
                Some(r) => r,
                None => {
                    if self.debug {
                        diag_debug!(
                            diag,
                            "GitDiffFilter: the file is not present in the git diff. Skipping mutant: {}:{}:{}",
                            file_path,
                            line,
                            column
                        );
                    }
                    return FilterDecision::Skip;
                }
            }
        };

        // Check if line is in any changed range
        for range in ranges {
            if range.contains(line) {
                if self.debug {
                    diag_debug!(
                        diag,
                        "GitDiffFilter: allowing mutant: {}:{}:{}",
                        file_path,
                        line,
                        column
                    );
                }
                return FilterDecision::Keep;
            }
        }

        if self.debug {
            diag_debug!(
                diag,
                "GitDiffFilter: skipping mutant: {}:{}:{}",
                file_path,
                line,
                column
            );
        }
        FilterDecision::Skip
    }

    fn name(&self) -> &str {
        "git diff"
    }
}

/// Read git diff using libgit2
fn read_git_diff(
    diag: &MullDiagnostics,
    git_project_root: &str,
    git_diff_ref: &str,
) -> GitDiffInfo {
    let mut diff_info: GitDiffInfo = HashMap::new();

    // Open the repository
    let repo = match Repository::open(git_project_root) {
        Ok(r) => r,
        Err(e) => {
            // Match the C++ warning format for compatibility with existing tests
            diag_warning!(
                diag,
                "GitDiffReader: cannot get git diff information. Received output: Not a git repository ({})",
                e
            );
            return diff_info;
        }
    };

    // Get the base commit (the ref to diff against)
    let base_commit = match repo.revparse_single(git_diff_ref) {
        Ok(obj) => match obj.peel_to_commit() {
            Ok(c) => c,
            Err(e) => {
                diag_warning!(
                    diag,
                    "GitDiffFilter: cannot resolve '{}' to a commit: {}",
                    git_diff_ref,
                    e
                );
                return diff_info;
            }
        },
        Err(e) => {
            diag_warning!(
                diag,
                "GitDiffFilter: cannot find ref '{}': {}",
                git_diff_ref,
                e
            );
            return diff_info;
        }
    };

    // Get tree for base commit
    let base_tree = match base_commit.tree() {
        Ok(t) => t,
        Err(e) => {
            diag_warning!(
                diag,
                "GitDiffFilter: cannot get tree for base commit: {}",
                e
            );
            return diff_info;
        }
    };

    // Generate diff with no context lines (like -U0)
    // Compare base tree to working directory (includes uncommitted changes)
    let mut diff_opts = DiffOptions::new();
    diff_opts.context_lines(0);

    let diff = match repo.diff_tree_to_workdir(Some(&base_tree), Some(&mut diff_opts)) {
        Ok(d) => d,
        Err(e) => {
            diag_warning!(diag, "GitDiffFilter: cannot compute diff: {}", e);
            return diff_info;
        }
    };

    // Extract line ranges from diff hunks
    let workdir = repo.workdir().map(|p| p.to_path_buf());

    let _ = diff.foreach(
        &mut |_delta, _progress| true, // file callback
        None,                          // binary callback
        Some(&mut |delta, hunk| {
            // hunk callback - extract line ranges
            if let Some(new_file) = delta.new_file().path() {
                // Build absolute path
                let abs_path = if let Some(ref wd) = workdir {
                    wd.join(new_file)
                } else {
                    new_file.to_path_buf()
                };

                let abs_path_str = abs_path
                    .canonicalize()
                    .unwrap_or(abs_path.clone())
                    .to_string_lossy()
                    .to_string();

                let new_start = hunk.new_start();
                let new_lines = hunk.new_lines();

                // Skip deletions (0 new lines)
                if new_lines > 0 {
                    diff_info.entry(abs_path_str).or_default().push(LineRange {
                        start: new_start,
                        count: new_lines,
                    });
                }
            }
            true
        }),
        None, // line callback
    );

    diff_info
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_line_range_contains() {
        let range = LineRange {
            start: 10,
            count: 5,
        }; // lines 10-14
        assert!(!range.contains(9));
        assert!(range.contains(10));
        assert!(range.contains(12));
        assert!(range.contains(14));
        assert!(!range.contains(15));
    }
}
