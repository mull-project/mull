use crate::{find_by_path_suffix, FilterDecision, MutantFilter};
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
    project_root: String,
    debug: bool,
}

impl GitDiffFilter {
    /// Create a new git diff filter by reading diff from git
    pub fn from_git_diff(
        diag: &MullDiagnostics,
        git_project_root: &str,
        git_diff_ref: &str,
        git_diff_paths: &[String],
        debug: bool,
    ) -> Self {
        // Expand to absolute path like C++ version does
        let abs_project_root = Path::new(git_project_root)
            .canonicalize()
            .map(|p| p.to_string_lossy().to_string())
            .unwrap_or_else(|_| git_project_root.to_string());

        let mut message = format!(
            "Incremental testing using Git Diff is enabled.\n- Git ref: {}\n- Git project root: {}",
            git_diff_ref, abs_project_root
        );
        if !git_diff_paths.is_empty() {
            message.push_str(&format!(
                "\n- Git diff paths: {}",
                git_diff_paths.join(", ")
            ));
        }
        diag_info!(diag, "{}", message);
        let diff_info = read_git_diff(diag, &abs_project_root, git_diff_ref, git_diff_paths);
        Self {
            diff_info,
            project_root: abs_project_root,
            debug,
        }
    }

    /// Create an empty filter (no filtering)
    pub fn empty() -> Self {
        Self {
            diff_info: HashMap::new(),
            project_root: String::new(),
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
        // A file under the project root and not in the diff has no changes
        let in_project = Path::new(file_path).starts_with(&self.project_root);
        let ranges = match self.diff_info.get(file_path).or_else(|| {
            (!in_project)
                .then(|| find_by_path_suffix(&self.diff_info, file_path))
                .flatten()
        }) {
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
    git_diff_paths: &[String],
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
    diff_opts
        .context_lines(0)
        .include_untracked(true)
        .recurse_untracked_dirs(true)
        .show_untracked_content(true);
    for path in git_diff_paths {
        diff_opts.pathspec(path);
    }

    // The index tells files added since the ref from the untracked ones
    let diff = match repo.diff_tree_to_workdir_with_index(Some(&base_tree), Some(&mut diff_opts)) {
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
    use git2::{IndexAddOption, Oid, Signature};
    use mull_state::SourceRange;
    use std::path::PathBuf;

    fn quiet_diag() -> MullDiagnostics {
        MullDiagnostics::with_writer(Box::new(std::io::sink()), false)
    }

    fn mutant_at(filepath: &str, line: u64) -> Mutant {
        Mutant {
            identifier: format!("cxx_add_to_sub:{}:{}:1:{}:2", filepath, line, line),
            mutator: "cxx_add_to_sub".into(),
            location: SourceRange::new(filepath.into(), line, 1, line, 2),
            replacement: String::new(),
        }
    }

    fn filter_with(project_root: &str, files: &[(&str, &[(u32, u32)])]) -> GitDiffFilter {
        let diff_info = files
            .iter()
            .map(|(path, ranges)| {
                let ranges = ranges
                    .iter()
                    .map(|&(start, count)| LineRange { start, count })
                    .collect();
                (path.to_string(), ranges)
            })
            .collect();
        GitDiffFilter {
            diff_info,
            project_root: project_root.into(),
            debug: false,
        }
    }

    fn keeps(filter: &GitDiffFilter, path: &str, line: u64) -> bool {
        filter.filter_decision(&mutant_at(path, line), &quiet_diag(), false) == FilterDecision::Keep
    }

    /// A git repository in a fresh temporary directory
    struct Sandbox {
        root: PathBuf,
        repo: Repository,
    }

    impl Sandbox {
        fn new(name: &str) -> Self {
            let root = std::env::temp_dir().join(format!(
                "mull-git-diff-filter-{}-{}",
                std::process::id(),
                name
            ));
            let _ = std::fs::remove_dir_all(&root);
            std::fs::create_dir_all(&root).unwrap();
            let repo = Repository::init(&root).unwrap();
            Self { root, repo }
        }

        fn write(&self, path: &str, content: &str) {
            let path = self.root.join(path);
            std::fs::create_dir_all(path.parent().unwrap()).unwrap();
            std::fs::write(path, content).unwrap();
        }

        fn remove(&self, path: &str) {
            std::fs::remove_file(self.root.join(path)).unwrap();
        }

        fn stage_all(&self) {
            let mut index = self.repo.index().unwrap();
            index.add_all(["*"], IndexAddOption::DEFAULT, None).unwrap();
            index.update_all(["*"], None).unwrap();
            index.write().unwrap();
        }

        fn commit(&self) -> Oid {
            self.stage_all();
            let mut index = self.repo.index().unwrap();
            let tree = self.repo.find_tree(index.write_tree().unwrap()).unwrap();
            let signature = Signature::now("Mull", "mull@example.com").unwrap();
            let parent = self.repo.head().ok().map(|h| h.peel_to_commit().unwrap());
            let parents: Vec<_> = parent.iter().collect();
            self.repo
                .commit(
                    Some("HEAD"),
                    &signature,
                    &signature,
                    "commit",
                    &tree,
                    &parents,
                )
                .unwrap()
        }

        fn canonical(&self, path: &str) -> String {
            self.root
                .canonicalize()
                .unwrap()
                .join(path)
                .to_string_lossy()
                .to_string()
        }

        fn filter(&self, git_ref: &str, paths: &[&str]) -> GitDiffFilter {
            let paths: Vec<String> = paths.iter().map(|p| p.to_string()).collect();
            GitDiffFilter::from_git_diff(
                &quiet_diag(),
                &self.root.to_string_lossy(),
                git_ref,
                &paths,
                false,
            )
        }

        fn ranges(filter: &GitDiffFilter) -> HashMap<String, Vec<(u32, u32)>> {
            filter
                .diff_info
                .iter()
                .map(|(path, ranges)| {
                    let mut ranges: Vec<_> = ranges.iter().map(|r| (r.start, r.count)).collect();
                    ranges.sort();
                    (path.clone(), ranges)
                })
                .collect()
        }
    }

    impl Drop for Sandbox {
        fn drop(&mut self) {
            let _ = std::fs::remove_dir_all(&self.root);
        }
    }

    fn lines(count: u32) -> String {
        (1..=count).map(|i| format!("line {}\n", i)).collect()
    }

    fn replace_lines(content: &str, replaced: &[usize]) -> String {
        content
            .lines()
            .enumerate()
            .map(|(i, line)| {
                if replaced.contains(&(i + 1)) {
                    format!("{} changed\n", line)
                } else {
                    format!("{}\n", line)
                }
            })
            .collect()
    }

    #[test]
    fn empty_diff_skips_everything() {
        let filter = filter_with("/src", &[]);
        assert!(!keeps(&filter, "/src/main.cpp", 1));
        assert!(!keeps(&filter, "/elsewhere/main.cpp", 1));
    }

    #[test]
    fn keeps_only_lines_inside_the_hunks() {
        let filter = filter_with("/src", &[("/src/main.cpp", &[(3, 1), (10, 3)])]);
        assert!(!keeps(&filter, "/src/main.cpp", 2));
        assert!(keeps(&filter, "/src/main.cpp", 3));
        assert!(!keeps(&filter, "/src/main.cpp", 4));
        assert!(!keeps(&filter, "/src/main.cpp", 9));
        assert!(keeps(&filter, "/src/main.cpp", 10));
        assert!(keeps(&filter, "/src/main.cpp", 12));
        assert!(!keeps(&filter, "/src/main.cpp", 13));
    }

    #[test]
    fn file_in_the_project_does_not_match_on_suffix() {
        let filter = filter_with("/src", &[("/src/a/util.cpp", &[(1, 10)])]);
        assert!(!keeps(&filter, "/src/b/util.cpp", 2));
        assert!(!keeps(&filter, "/src/util.cpp", 2));
    }

    #[test]
    fn file_outside_of_the_project_matches_on_the_longest_suffix() {
        let filter = filter_with(
            "/src",
            &[
                ("/src/a/util.cpp", &[(1, 10)]),
                ("/src/b/util.cpp", &[(20, 1)]),
            ],
        );
        assert!(keeps(&filter, "/sandbox/a/util.cpp", 2));
        assert!(!keeps(&filter, "/sandbox/b/util.cpp", 2));
        assert!(keeps(&filter, "/sandbox/b/util.cpp", 20));
    }

    #[test]
    fn file_outside_of_the_project_with_an_ambiguous_name_is_skipped() {
        let filter = filter_with(
            "/src",
            &[
                ("/src/a/util.cpp", &[(1, 10)]),
                ("/src/b/util.cpp", &[(1, 10)]),
            ],
        );
        assert!(!keeps(&filter, "/sandbox/util.cpp", 2));
    }

    #[test]
    fn file_outside_of_the_project_and_not_in_the_diff_is_skipped() {
        let filter = filter_with("/src", &[("/src/a/util.cpp", &[(1, 10)])]);
        assert!(!keeps(&filter, "/usr/include/vector", 2));
    }

    #[test]
    fn modified_lines_are_separate_hunks() {
        let sandbox = Sandbox::new("modified");
        let original = lines(10);
        sandbox.write("main.cpp", &original);
        sandbox.commit();
        sandbox.write("main.cpp", &replace_lines(&original, &[3, 7, 8]));

        let filter = sandbox.filter("HEAD", &[]);
        assert_eq!(
            Sandbox::ranges(&filter),
            HashMap::from([(sandbox.canonical("main.cpp"), vec![(3, 1), (7, 2)])])
        );
    }

    #[test]
    fn inserted_lines_are_new_lines_only() {
        let sandbox = Sandbox::new("inserted");
        sandbox.write("main.cpp", &lines(5));
        sandbox.commit();
        sandbox.write("main.cpp", &(lines(4) + "new 1\nnew 2\nline 5\n"));

        let filter = sandbox.filter("HEAD", &[]);
        assert_eq!(
            Sandbox::ranges(&filter),
            HashMap::from([(sandbox.canonical("main.cpp"), vec![(5, 2)])])
        );
    }

    #[test]
    fn deleted_lines_and_files_have_no_ranges() {
        let sandbox = Sandbox::new("deleted");
        sandbox.write("main.cpp", &lines(5));
        sandbox.write("gone.cpp", &lines(5));
        sandbox.commit();
        sandbox.write("main.cpp", &(lines(2) + "line 4\nline 5\n"));
        sandbox.remove("gone.cpp");

        let filter = sandbox.filter("HEAD", &[]);
        assert!(Sandbox::ranges(&filter).is_empty());
    }

    #[test]
    fn new_files_are_entirely_new_whether_committed_staged_or_untracked() {
        let sandbox = Sandbox::new("new-files");
        sandbox.write("main.cpp", &lines(5));
        let base = sandbox.commit();
        sandbox.write("committed.cpp", &lines(2));
        sandbox.commit();
        sandbox.write("staged.cpp", &lines(3));
        sandbox.stage_all();
        sandbox.write("nested/dir/untracked.cpp", &lines(4));

        let filter = sandbox.filter(&base.to_string(), &[]);
        assert_eq!(
            Sandbox::ranges(&filter),
            HashMap::from([
                (sandbox.canonical("committed.cpp"), vec![(1, 2)]),
                (sandbox.canonical("staged.cpp"), vec![(1, 3)]),
                (sandbox.canonical("nested/dir/untracked.cpp"), vec![(1, 4)]),
            ])
        );
    }

    #[test]
    fn ignored_files_are_not_in_the_diff() {
        let sandbox = Sandbox::new("ignored");
        sandbox.write(".gitignore", "build/\n");
        sandbox.commit();
        sandbox.write("build/generated.cpp", &lines(3));

        let filter = sandbox.filter("HEAD", &[]);
        assert!(Sandbox::ranges(&filter).is_empty());
    }

    #[test]
    fn renamed_file_is_entirely_new() {
        let sandbox = Sandbox::new("renamed");
        sandbox.write("old.cpp", &lines(3));
        sandbox.commit();
        sandbox.remove("old.cpp");
        sandbox.write("new.cpp", &lines(3));

        let filter = sandbox.filter("HEAD", &[]);
        assert_eq!(
            Sandbox::ranges(&filter),
            HashMap::from([(sandbox.canonical("new.cpp"), vec![(1, 3)])])
        );
    }

    #[test]
    fn committed_and_uncommitted_changes_since_the_ref_are_combined() {
        let sandbox = Sandbox::new("combined");
        let original = lines(10);
        sandbox.write("main.cpp", &original);
        let base = sandbox.commit();
        sandbox.write("main.cpp", &replace_lines(&original, &[2]));
        sandbox.commit();
        sandbox.write("main.cpp", &replace_lines(&original, &[2, 8]));

        let filter = sandbox.filter(&base.to_string(), &[]);
        assert_eq!(
            Sandbox::ranges(&filter),
            HashMap::from([(sandbox.canonical("main.cpp"), vec![(2, 1), (8, 1)])])
        );
    }

    #[test]
    fn diff_paths_restrict_the_diff() {
        let sandbox = Sandbox::new("diff-paths");
        let original = lines(3);
        sandbox.write("a/util.cpp", &original);
        sandbox.write("b/util.cpp", &original);
        sandbox.commit();
        sandbox.write("a/util.cpp", &replace_lines(&original, &[2]));
        sandbox.write("b/util.cpp", &replace_lines(&original, &[2]));
        sandbox.write("a/untracked.cpp", &original);
        sandbox.write("c/untracked.cpp", &original);

        let filter = sandbox.filter("HEAD", &["a"]);
        assert_eq!(
            Sandbox::ranges(&filter),
            HashMap::from([
                (sandbox.canonical("a/util.cpp"), vec![(2, 1)]),
                (sandbox.canonical("a/untracked.cpp"), vec![(1, 3)]),
            ])
        );
    }

    #[test]
    fn project_root_through_a_symlink_is_canonical() {
        let sandbox = Sandbox::new("symlinked");
        let original = lines(3);
        sandbox.write("main.cpp", &original);
        sandbox.commit();
        sandbox.write("main.cpp", &replace_lines(&original, &[2]));
        let link = sandbox.root.with_extension("link");
        let _ = std::fs::remove_file(&link);
        std::os::unix::fs::symlink(&sandbox.root, &link).unwrap();

        let filter = GitDiffFilter::from_git_diff(
            &quiet_diag(),
            &link.to_string_lossy(),
            "HEAD",
            &[],
            false,
        );
        let _ = std::fs::remove_file(&link);
        assert_eq!(
            Path::new(&filter.project_root),
            sandbox.root.canonicalize().unwrap()
        );
        assert!(keeps(&filter, &sandbox.canonical("main.cpp"), 2));
        assert!(!keeps(&filter, &sandbox.canonical("main.cpp"), 1));
    }

    #[test]
    fn unresolvable_ref_skips_everything() {
        let sandbox = Sandbox::new("bad-ref");
        sandbox.write("main.cpp", &lines(3));
        sandbox.commit();
        sandbox.write("main.cpp", &lines(4));

        let filter = sandbox.filter("does-not-exist", &[]);
        assert!(filter.diff_info.is_empty());
        assert!(!keeps(&filter, &sandbox.canonical("main.cpp"), 4));
    }

    #[test]
    fn directory_that_is_not_a_repository_skips_everything() {
        let root = std::env::temp_dir().join(format!(
            "mull-git-diff-filter-{}-not-a-repo",
            std::process::id()
        ));
        std::fs::create_dir_all(&root).unwrap();
        let filter = GitDiffFilter::from_git_diff(
            &quiet_diag(),
            &root.to_string_lossy(),
            "HEAD",
            &[],
            false,
        );
        let _ = std::fs::remove_dir_all(&root);
        assert!(filter.diff_info.is_empty());
    }

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
