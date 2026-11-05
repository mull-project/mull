use mull_core::{diag_warning, MullDiagnostics};
use mull_state::SourceRange;
use std::collections::HashMap;
use unicode_width::UnicodeWidthChar;

/// Byte offset of a 1-based column, columns count bytes like clang does
pub(crate) fn column_offset(line: &str, column: u64) -> Option<usize> {
    let offset = column.checked_sub(1)? as usize;
    line.is_char_boundary(offset).then_some(offset)
}

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

    fn build_caret_line(&self, line: &str, offset: usize) -> String {
        let mut caret = String::with_capacity(offset + 1);
        for ch in line[..offset].chars() {
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
        let Some(offset) =
            column_offset(line, location.begin_column).filter(|&offset| offset < line.len())
        else {
            diag_warning!(
                diag,
                "Requested column past the line end or inside a character: filepath: '{}', line: {}, column: {}, byte count: {}",
                location.filepath,
                location.begin_line,
                location.begin_column,
                line.len()
            );
            return None;
        };

        let caret = self.build_caret_line(line, offset);
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
            let begin = column_offset(line, range.begin_column)?;
            let end = column_offset(line, range.end_column)?;
            line.get(begin..end).map(String::from)
        } else {
            let first_line =
                self.get_source_line(diag, range.filepath.clone(), range.begin_line)?;
            let last_line = self.get_source_line(diag, range.filepath.clone(), range.end_line)?;
            let first_column = column_offset(first_line, range.begin_column)?;
            let last_column = column_offset(last_line, range.end_column)?;
            let mut result = first_line[first_column..].to_string() + "\n";
            for i in range.begin_line.checked_add(1)?..range.end_line {
                result += &(self
                    .get_source_line(diag, range.filepath.clone(), i)?
                    .to_owned()
                    + "\n");
            }
            result += &last_line[..last_column];
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

        // 中 is 3 bytes
        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 8, 1, 9))
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
            sm.get_source(&diag, &fake_range(1, 4, 1, 9)).unwrap(),
            " 中x".to_string()
        );
        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 5, 1, 8)).unwrap(),
            "中".to_string()
        );
        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 12, 1, 14)).unwrap(),
            "42".to_string()
        )
    }

    #[test]
    fn two_line_source() {
        let diag = MullDiagnostics::new();
        let mut sm = fake_sm(vec!["let 👋x = 42;", "let 中y = 10;"]);

        assert_eq!(
            sm.get_source(&diag, &fake_range(1, 4, 2, 8)).unwrap(),
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
            sm.get_source(&diag, &fake_range(1, 4, 3, 8)).unwrap(),
            indoc!(
                " 👋x = 42;
                let üy = 20;
                let 中"
            )
        );
    }

    #[test]
    fn columns_are_bytes_after_non_ascii() {
        let diag = MullDiagnostics::new();
        let line = "const char *s = \"üü\"; return a + b;";
        let mut sm = fake_sm(vec![line]);
        let plus = line.find('+').unwrap() as u64 + 1;

        assert_eq!(
            sm.get_source(&diag, &fake_range(1, plus, 1, plus + 1))
                .unwrap(),
            "+"
        );
        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, plus, 1, plus + 1))
                .unwrap(),
            format!("{}\n{}^", line, " ".repeat(31))
        );
    }

    #[test]
    fn column_inside_a_character_is_rejected() {
        let diag = MullDiagnostics::with_writer(Box::new(std::io::sink()), false);
        let mut sm = fake_sm(vec!["let 中x = 42;"]);

        assert_eq!(sm.get_source(&diag, &fake_range(1, 6, 1, 8)), None);
        assert_eq!(
            sm.get_source_line_with_caret(&diag, &fake_range(1, 6, 1, 8)),
            None
        );
    }
}
