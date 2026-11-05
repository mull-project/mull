use crate::find_by_path_suffix;
use std::collections::HashMap;

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
    fn exact_path_wins_over_suffix() {
        let ranges = ranges(&[
            ("/sandbox/util.cpp", Span::lines(1, 1)),
            ("/src/util.cpp", Span::lines(5, 5)),
        ]);
        assert_eq!(ranges.contains("/src/util.cpp", 5, 1, true), Some(true));
        assert_eq!(ranges.contains("/src/util.cpp", 1, 1, true), Some(false));
    }
}
