use mull_core::{diag_warning, MullDiagnostics};
use mull_state::SourceRange;
use std::collections::HashMap;
use unicode_width::UnicodeWidthChar;
#[derive(Debug, Default)]
pub struct SourceManager {
    cache: HashMap<String, Vec<String>>,
}

impl SourceManager {
    fn ensure_cached(&mut self, diag: &MullDiagnostics, filepath: &str) {
        if !self.cache.contains_key(filepath) {
            match std::fs::read_to_string(filepath) {
                Ok(contents) => {
                    let lines: Vec<String> = contents.lines().map(String::from).collect();
                    self.cache.insert(filepath.to_string(), lines);
                }
                Err(err) => {
                    diag_warning!(diag, "Cannot read source file '{}': {}", filepath, err);
                }
            }
        }
    }

    fn get_source_line(&self, diag: &MullDiagnostics, filepath: String, line: u64) -> Option<&str> {
        let lines = self.cache.get(&filepath)?;
        let index = line.checked_sub(1)? as usize;
        if index >= lines.len() {
            diag_warning!(
                diag,
                "Requested line {} from a file with {} line(s), filepath: '{}'",
                line,
                lines.len(),
                filepath
            );
            return None;
        }
        Some(lines[index].as_str())
    }

    fn build_caret_line(&self, line: &str, column: usize) -> String {
        let mut caret = String::with_capacity(column + 1);
        for ch in line.chars().take(column - 1) {
            match ch {
                '\t' => caret.push('\t'),
                _ => {
                    let width = ch.width().unwrap_or(0);
                    for _ in 0..width {
                        caret.push(' ');
                    }
                }
            }
        }
        caret.push('^');
        caret
    }

    pub fn get_source_line_with_caret(
        &mut self,
        diag: &MullDiagnostics,
        location: &SourceRange,
    ) -> Option<String> {
        self.ensure_cached(diag, &location.filepath);
        let line = self.get_source_line(diag, location.filepath.clone(), location.begin_line)?;
        let char_count = line.chars().count();
        let col = location.begin_column as usize;

        if col == 0 || col > char_count {
            diag_warning!(
                diag,
                "Requested column past the line end: filepath: '{}', line: {}, column: {}, char count: {}",
                location.filepath,
                location.begin_line,
                location.begin_column,
                char_count
            );
            return None;
        }

        let caret = self.build_caret_line(line, col);
        Some(format!("{}\n{}", line, caret))
    }

    pub fn get_source_lines(
        &mut self,
        diag: &MullDiagnostics,
        range: &SourceRange,
    ) -> Option<Vec<String>> {
        self.ensure_cached(diag, &range.filepath);
        let mut result = Vec::new();
        for line_num in range.begin_line..=range.end_line {
            let line = self.get_source_line(diag, range.filepath.clone(), line_num)?;
            result.push(line.to_string());
        }
        Some(result)
    }

    pub fn get_source(&mut self, diag: &MullDiagnostics, range: &SourceRange) -> Option<String> {
        self.ensure_cached(diag, &range.filepath);
        if range.begin_line == range.end_line {
            let line = self.get_source_line(diag, range.filepath.clone(), range.begin_line)?;
            let column = range.begin_column.checked_sub(1)? as usize;
            Some(
                line.chars()
                    .skip(column)
                    .take((range.end_column.checked_sub(1)? as usize) - column)
                    .collect(),
            )
        } else {
            let first_line =
                self.get_source_line(diag, range.filepath.clone(), range.begin_line)?;
            let last_line = self.get_source_line(diag, range.filepath.clone(), range.end_line)?;
            let first_column = range.begin_column.checked_sub(1)? as usize;
            let last_column = range.end_column.checked_sub(1)? as usize;
            let mut result = first_line.chars().skip(first_column).collect::<String>() + "\n";
            for i in range.begin_line.checked_add(1)?..range.end_line {
                result += &(self
                    .get_source_line(diag, range.filepath.clone(), i)?
                    .to_owned()
                    + "\n");
            }
            result += &last_line.chars().take(last_column).collect::<String>();
            Some(result)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use indoc::indoc;

    fn fake_sm(content: Vec<&str>) -> SourceManager {
        let mut sm = SourceManager::default();
        sm.cache.insert(
            "fakefile".to_string(),
            content.iter().map(|s| s.to_string()).collect(),
        );
        sm
    }
    fn fake_range(
        begin_line: u64,
        begin_column: u64,
        end_line: u64,
        end_column: u64,
    ) -> SourceRange {
        SourceRange::new(
            "fakefile".to_string(),
            begin_line,
            begin_column,
            end_line,
            end_column,
        )
    }

    #[test]
    fn emojis() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let 👋 = 42;"]);

        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 5, 1, 6))
                .unwrap(),
            indoc!(
                "
                let 👋 = 42;
                    ^       "
            )
            .trim_end()
        )
    }

    #[test]
    fn caret_ascii() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let x = 42;"]);
        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 5, 1, 6))
                .unwrap(),
            indoc!(
                "
                let x = 42;
                    ^       "
            )
            .trim_end()
        )
    }

    #[test]
    fn tab() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["\tlet x = 42;"]);

        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 6, 1, 7))
                .unwrap(),
            indoc!(
                "
                \tlet x = 42;
                \t    ^        "
            )
            .trim_end()
        )
    }

    #[test]
    fn begin() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let x = 42;"]);

        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 1, 1, 2))
                .unwrap(),
            indoc!(
                "
                let x = 42;
                ^         "
            )
            .trim_end()
        )
    }

    #[test]
    fn end() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let x = 42;"]);

        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 11, 1, 12))
                .unwrap(),
            indoc!(
                "
                let x = 42;
                          ^"
            )
            .trim_end()
        )
    }

    #[test]
    fn wide() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let 中x = 42;"]);

        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 6, 1, 7))
                .unwrap(),
            indoc!(
                "
                let 中x = 42;
                      ^"
            )
            .trim_end()
        )
    }

    #[test]
    fn single_line_source() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let 中x = 42;"]);

        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 4, 1, 7)).unwrap(),
            " 中x".to_string()
        );
        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 5, 1, 6)).unwrap(),
            "中".to_string()
        );
        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 10, 1, 12)).unwrap(),
            "42".to_string()
        )
    }

    #[test]
    fn two_line_source() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let 👋x = 42;", "let 中y = 10;"]);

        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 4, 2, 6)).unwrap(),
            indoc!(
                " 👋x = 42;
                let 中"
            )
        );
    }

    #[test]
    fn multiline_source() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let 👋x = 42;", "let üy = 20;", "let 中y = 10;"]);

        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 4, 3, 6)).unwrap(),
            indoc!(
                " 👋x = 42;
                let üy = 20;
                let 中"
            )
        );
    }
}
