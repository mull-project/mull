use crate::file_ranges::{FileRanges, Span};
use crate::{FilterDecision, MutantFilter};
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_debug, diag_info, diag_warning};
use mull_state::Mutant;
use std::path::Path;

/// Filter that keeps only mutants starting on the lines listed in a ranges file
///
/// Each line of the file is `<path>:<first>-<last>` or `<path>:<line>`, blank lines and lines
/// starting with `#` are ignored. Relative paths are relative to the ranges file.
pub struct MutationRangesFilter {
    ranges: FileRanges,
    debug: bool,
}

impl MutationRangesFilter {
    pub fn from_file(diag: &MullDiagnostics, ranges_path: &str, debug: bool) -> Self {
        diag_info!(
            diag,
            "Restricting mutations to the ranges in {}",
            ranges_path
        );
        let ranges = match std::fs::read_to_string(ranges_path) {
            Ok(content) => {
                // Not canonicalized: the ranges file may be a symlink to another directory
                let base = Path::new(ranges_path).parent().unwrap_or(Path::new(""));
                match parse_ranges(&content, base) {
                    Ok(ranges) if ranges.is_empty() => {
                        diag_warning!(
                            diag,
                            "mutationRanges: '{}' has no ranges, every mutant is skipped",
                            ranges_path
                        );
                        ranges
                    }
                    Ok(ranges) => ranges,
                    Err(e) => {
                        diag_warning!(
                            diag,
                            "mutationRanges: cannot parse '{}': {}, every mutant is skipped",
                            ranges_path,
                            e
                        );
                        FileRanges::default()
                    }
                }
            }
            Err(e) => {
                diag_warning!(
                    diag,
                    "mutationRanges: cannot read '{}': {}, every mutant is skipped",
                    ranges_path,
                    e
                );
                FileRanges::default()
            }
        };
        Self { ranges, debug }
    }
}

impl MutantFilter for MutationRangesFilter {
    fn filter_decision(
        &self,
        mutant: &Mutant,
        diag: &MullDiagnostics,
        _debug: bool,
    ) -> FilterDecision {
        let location = &mutant.location;
        // Mutant paths are real paths, a suffix match could pick a file with the same name
        let keep = self.ranges.contains(
            &location.filepath,
            location.begin_line,
            location.begin_column,
            false,
        ) == Some(true);
        if self.debug {
            diag_debug!(
                diag,
                "MutationRangesFilter: {} mutant: {}:{}:{}",
                if keep { "allowing" } else { "skipping" },
                location.filepath,
                location.begin_line,
                location.begin_column
            );
        }
        if keep {
            FilterDecision::Keep
        } else {
            FilterDecision::Skip
        }
    }

    fn name(&self) -> &str {
        "mutation ranges"
    }
}

fn parse_ranges(content: &str, base: &Path) -> Result<FileRanges, String> {
    let mut ranges = FileRanges::default();
    for (index, line) in content.lines().enumerate() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        let (path, first, last) = parse_entry(line).ok_or_else(|| {
            format!(
                "line {}: expected '<path>:<first>-<last>' or '<path>:<line>', got '{}'",
                index + 1,
                line
            )
        })?;
        ranges.insert(&resolve(base, path), Span::lines(first, last));
    }
    Ok(ranges.finish())
}

fn parse_entry(line: &str) -> Option<(&str, u64, u64)> {
    // The path can contain ':', the line numbers cannot
    let (path, lines) = line.rsplit_once(':')?;
    let path = path.trim();
    let (first, last) = match lines.split_once('-') {
        Some((first, last)) => (first.trim().parse().ok()?, last.trim().parse().ok()?),
        None => {
            let line = lines.trim().parse().ok()?;
            (line, line)
        }
    };
    (!path.is_empty() && first >= 1 && first <= last).then_some((path, first, last))
}

fn resolve(base: &Path, path: &str) -> String {
    let path = base.join(path);
    path.canonicalize()
        .unwrap_or(path)
        .to_string_lossy()
        .into_owned()
}

#[cfg(test)]
mod tests {
    use super::*;
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

    fn keeps(filter: &MutationRangesFilter, path: &str, line: u64) -> bool {
        filter.filter_decision(&mutant_at(path, line), &quiet_diag(), false) == FilterDecision::Keep
    }

    /// A fresh temporary directory, canonicalized
    struct Sandbox {
        root: PathBuf,
    }

    impl Sandbox {
        fn new(name: &str) -> Self {
            let root = std::env::temp_dir().join(format!(
                "mull-mutation-ranges-filter-{}-{}",
                std::process::id(),
                name
            ));
            let _ = std::fs::remove_dir_all(&root);
            std::fs::create_dir_all(&root).unwrap();
            Self {
                root: root.canonicalize().unwrap(),
            }
        }

        fn write(&self, path: &str, content: &str) -> String {
            let path = self.root.join(path);
            std::fs::create_dir_all(path.parent().unwrap()).unwrap();
            std::fs::write(&path, content).unwrap();
            path.to_string_lossy().into_owned()
        }

        fn path(&self, path: &str) -> String {
            self.root.join(path).to_string_lossy().into_owned()
        }

        fn filter(&self, ranges_path: &str) -> MutationRangesFilter {
            MutationRangesFilter::from_file(&quiet_diag(), ranges_path, false)
        }
    }

    impl Drop for Sandbox {
        fn drop(&mut self) {
            let _ = std::fs::remove_dir_all(&self.root);
        }
    }

    #[test]
    fn parses_ranges_single_lines_comments_and_blank_lines() {
        let ranges = parse_ranges(
            "# comment\n\n/src/a.cpp:3-5\r\n  /src/a.cpp:10  \n/src/b.cpp: 7 - 8\n",
            Path::new("/"),
        )
        .unwrap();
        assert_eq!(
            ranges.spans("/src/a.cpp", false).unwrap(),
            &[Span::lines(3, 5), Span::lines(10, 10)]
        );
        assert_eq!(
            ranges.spans("/src/b.cpp", false).unwrap(),
            &[Span::lines(7, 8)]
        );
    }

    #[test]
    fn path_can_contain_colons() {
        let ranges = parse_ranges("/src/a:b.cpp:3-5\n", Path::new("/")).unwrap();
        assert_eq!(
            ranges.spans("/src/a:b.cpp", false).unwrap(),
            &[Span::lines(3, 5)]
        );
    }

    #[test]
    fn rejects_malformed_entries() {
        for line in [
            "/src/a.cpp",
            "/src/a.cpp:",
            ":3-5",
            "/src/a.cpp:x",
            "/src/a.cpp:3-",
            "/src/a.cpp:-5",
            "/src/a.cpp:0-5",
            "/src/a.cpp:5-3",
            "/src/a.cpp:3-5,7-8",
        ] {
            let error = parse_ranges(&format!("# ok\n{}\n", line), Path::new("/"))
                .err()
                .unwrap_or_else(|| panic!("'{}' parsed", line));
            assert!(error.starts_with("line 2:"), "{}", error);
        }
    }

    #[test]
    fn keeps_only_mutants_starting_inside_the_ranges() {
        let sandbox = Sandbox::new("keeps");
        let source = sandbox.write("src/a.cpp", "");
        let ranges = sandbox.write("ranges", &format!("{}:3-5\n{}:10\n", source, source));
        let filter = sandbox.filter(&ranges);
        assert!(!keeps(&filter, &source, 2));
        assert!(keeps(&filter, &source, 3));
        assert!(keeps(&filter, &source, 5));
        assert!(!keeps(&filter, &source, 6));
        assert!(keeps(&filter, &source, 10));
        assert!(!keeps(&filter, &source, 11));
    }

    #[test]
    fn unlisted_file_with_the_same_name_is_skipped() {
        let sandbox = Sandbox::new("same-name");
        let listed = sandbox.write("a/util.cpp", "");
        let other = sandbox.write("b/util.cpp", "");
        let ranges = sandbox.write("ranges", &format!("{}:1-10\n", listed));
        let filter = sandbox.filter(&ranges);
        assert!(keeps(&filter, &listed, 2));
        assert!(!keeps(&filter, &other, 2));
    }

    #[test]
    fn relative_paths_are_relative_to_the_ranges_file() {
        let sandbox = Sandbox::new("relative");
        let source = sandbox.write("src/a.cpp", "");
        let ranges = sandbox.write("config/ranges", "../src/a.cpp:2\n");
        let filter = sandbox.filter(&ranges);
        assert!(keeps(&filter, &source, 2));
    }

    #[test]
    fn relative_paths_are_relative_to_the_symlink_not_its_target() {
        let sandbox = Sandbox::new("symlink");
        let source = sandbox.write("work/src/a.cpp", "");
        let target = sandbox.write("elsewhere/ranges", "src/a.cpp:2\n");
        let link = sandbox.path("work/ranges");
        std::os::unix::fs::symlink(&target, &link).unwrap();
        let filter = sandbox.filter(&link);
        assert!(keeps(&filter, &source, 2));
    }

    #[test]
    fn listed_paths_are_canonicalized() {
        let sandbox = Sandbox::new("canonical");
        let source = sandbox.write("src/a.cpp", "");
        std::os::unix::fs::symlink(sandbox.path("src"), sandbox.path("link")).unwrap();
        let ranges = sandbox.write("ranges", "link/./a.cpp:2\n");
        let filter = sandbox.filter(&ranges);
        assert!(keeps(&filter, &source, 2));
    }

    #[test]
    fn missing_listed_file_is_kept_as_written() {
        let sandbox = Sandbox::new("missing-source");
        let ranges = sandbox.write("ranges", "gone.cpp:2\n");
        let filter = sandbox.filter(&ranges);
        assert!(keeps(&filter, &sandbox.path("gone.cpp"), 2));
    }

    #[test]
    fn missing_empty_or_malformed_ranges_file_skips_everything() {
        let sandbox = Sandbox::new("bad-file");
        let source = sandbox.write("a.cpp", "");
        let empty = sandbox.write("empty", "# nothing\n\n");
        let malformed = sandbox.write("malformed", &format!("{}:2\n{}:x\n", source, source));
        for ranges in [sandbox.path("missing"), empty, malformed] {
            let filter = sandbox.filter(&ranges);
            assert!(!keeps(&filter, &source, 2), "{}", ranges);
        }
    }
}
