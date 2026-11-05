use std::collections::HashMap;
use std::path::Path;

/// Inclusive span between two (line, column) positions
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) struct Span {
    begin: (u64, u64),
    end: (u64, u64),
}

impl Span {
    pub(crate) fn new(begin: (u64, u64), end: (u64, u64)) -> Self {
        Self { begin, end }
    }

    /// Whole lines, any column
    pub(crate) fn lines(first: u64, last: u64) -> Self {
        Self::new((first, 0), (last, u64::MAX))
    }

    #[cfg(test)]
    pub(crate) fn line_range(&self) -> (u64, u64) {
        (self.begin.0, self.end.0)
    }

    pub(crate) fn contains(&self, line: u64, column: u64) -> bool {
        self.begin <= (line, column) && (line, column) <= self.end
    }
}

/// Spans per file path, sorted and merged by `finish` for binary search
#[derive(Debug, Default)]
pub(crate) struct FileRanges {
    by_path: HashMap<String, Vec<Span>>,
}

impl FileRanges {
    pub(crate) fn insert(&mut self, path: &str, span: Span) {
        self.by_path.entry(path.to_string()).or_default().push(span);
    }

    /// Makes the file known even without spans
    pub(crate) fn add_file(&mut self, path: &str) {
        self.by_path.entry(path.to_string()).or_default();
    }

    /// Sorts and merges overlapping spans; required before lookups
    pub(crate) fn finish(mut self) -> Self {
        for spans in self.by_path.values_mut() {
            spans.sort_by_key(|span| span.begin);
            let mut merged: Vec<Span> = Vec::with_capacity(spans.len());
            for span in spans.drain(..) {
                match merged.last_mut() {
                    Some(last) if span.begin <= last.end => last.end = last.end.max(span.end),
                    _ => merged.push(span),
                }
            }
            *spans = merged;
        }
        self
    }

    pub(crate) fn is_empty(&self) -> bool {
        self.by_path.is_empty()
    }

    #[cfg(test)]
    pub(crate) fn iter(&self) -> impl Iterator<Item = (&str, &[Span])> {
        self.by_path
            .iter()
            .map(|(path, spans)| (path.as_str(), spans.as_slice()))
    }

    /// Exact path first, then the longest path suffix if `suffix_fallback` is set
    pub(crate) fn spans(&self, path: &str, suffix_fallback: bool) -> Option<&[Span]> {
        self.by_path
            .get(path)
            .or_else(|| {
                suffix_fallback
                    .then(|| find_by_path_suffix(&self.by_path, path))
                    .flatten()
            })
            .map(Vec::as_slice)
    }

    /// `None` if the file is unknown
    pub(crate) fn contains(
        &self,
        path: &str,
        line: u64,
        column: u64,
        suffix_fallback: bool,
    ) -> Option<bool> {
        let spans = self.spans(path, suffix_fallback)?;
        let after = spans.partition_point(|span| span.begin <= (line, column));
        Some(after > 0 && spans[after - 1].contains(line, column))
    }
}

/// Find the entry sharing the longest path suffix with `path`. Handles cases where the entries
/// have different path prefixes (e.g., sandbox paths) than debug info. Ambiguous matches are `None`.
fn find_by_path_suffix<'a, T>(entries: &'a HashMap<String, T>, path: &str) -> Option<&'a T> {
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

#[cfg(test)]
mod tests {
    use super::*;

    fn ranges(entries: &[(&str, Span)]) -> FileRanges {
        let mut ranges = FileRanges::default();
        for (path, span) in entries {
            ranges.insert(path, *span);
        }
        ranges.finish()
    }

    #[test]
    fn line_span_ignores_columns() {
        let span = Span::lines(3, 5);
        assert!(!span.contains(2, u64::MAX));
        assert!(span.contains(3, 0));
        assert!(span.contains(5, u64::MAX));
        assert!(!span.contains(6, 0));
    }

    #[test]
    fn column_span_respects_columns_on_the_first_and_last_line() {
        let span = Span::new((3, 10), (5, 4));
        assert!(!span.contains(3, 9));
        assert!(span.contains(3, 10));
        assert!(span.contains(4, 1));
        assert!(span.contains(5, 4));
        assert!(!span.contains(5, 5));
    }

    #[test]
    fn overlapping_and_nested_spans_are_merged() {
        let ranges = ranges(&[
            ("/a.cpp", Span::lines(10, 20)),
            ("/a.cpp", Span::lines(1, 3)),
            ("/a.cpp", Span::lines(15, 25)),
            ("/a.cpp", Span::lines(2, 2)),
            ("/a.cpp", Span::lines(30, 30)),
        ]);
        assert_eq!(
            ranges.spans("/a.cpp", false).unwrap(),
            &[Span::lines(1, 3), Span::lines(10, 25), Span::lines(30, 30)]
        );
    }

    #[test]
    fn contains_finds_the_right_span() {
        let ranges = ranges(&[
            ("/a.cpp", Span::lines(10, 20)),
            ("/a.cpp", Span::lines(1, 3)),
            ("/a.cpp", Span::lines(30, 30)),
        ]);
        let contains = |line| ranges.contains("/a.cpp", line, 1, false);
        assert_eq!(contains(1), Some(true));
        assert_eq!(contains(3), Some(true));
        assert_eq!(contains(4), Some(false));
        assert_eq!(contains(9), Some(false));
        assert_eq!(contains(10), Some(true));
        assert_eq!(contains(20), Some(true));
        assert_eq!(contains(29), Some(false));
        assert_eq!(contains(30), Some(true));
        assert_eq!(contains(31), Some(false));
    }

    #[test]
    fn unknown_file_is_none() {
        let ranges = ranges(&[("/src/a/util.cpp", Span::lines(1, 3))]);
        assert_eq!(ranges.contains("/src/b/util.cpp", 1, 1, false), None);
        assert_eq!(ranges.contains("/src/b/util.cpp", 1, 1, true), Some(true));
        assert_eq!(ranges.contains("/src/b/main.cpp", 1, 1, true), None);
    }

    #[test]
    fn file_without_spans_is_known() {
        let mut ranges = FileRanges::default();
        ranges.add_file("/a.cpp");
        ranges.insert("/b.cpp", Span::lines(1, 1));
        ranges.add_file("/b.cpp");
        let ranges = ranges.finish();
        assert_eq!(ranges.contains("/a.cpp", 1, 1, false), Some(false));
        assert_eq!(ranges.contains("/b.cpp", 1, 1, false), Some(true));
    }

    #[test]
    fn exact_path_wins_over_suffix() {
        let ranges = ranges(&[
            ("/sandbox/util.cpp", Span::lines(1, 1)),
            ("/src/util.cpp", Span::lines(5, 5)),
        ]);
        assert_eq!(ranges.contains("/src/util.cpp", 5, 1, true), Some(true));
        assert_eq!(ranges.contains("/src/util.cpp", 1, 1, true), Some(false));
    }

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
