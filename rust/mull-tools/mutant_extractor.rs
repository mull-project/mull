use crate::dynamic_libraries::maybe_extract_fat_slice;
use mull_core::{diag_debug, diag_warning, diagnostics::MullDiagnostics};
use mull_state::Mutant;
use object::{Object, ObjectSection};
use std::collections::HashSet;
use std::fs;
use std::path::Path;

const MULL_MUTANTS_SECTION: &str = ".mull_mutants";

fn extract_from_file(diag: &MullDiagnostics, path: &str) -> Vec<String> {
    let data = match fs::read(path) {
        Ok(d) => d,
        Err(e) => {
            diag_warning!(diag, "Cannot read file '{}': {}", path, e);
            return Vec::new();
        }
    };

    let slice = maybe_extract_fat_slice(&data, diag);

    let file = match object::File::parse(slice) {
        Ok(f) => f,
        Err(e) => {
            diag_warning!(diag, "Cannot parse '{}': {}", path, e);
            return Vec::new();
        }
    };

    let Some(section) = file.section_by_name(MULL_MUTANTS_SECTION) else {
        return Vec::new();
    };

    let Ok(content) = section.data() else {
        diag_warning!(
            diag,
            "Cannot read section '{}' from '{}'",
            MULL_MUTANTS_SECTION,
            path
        );
        return Vec::new();
    };

    content
        .split(|&b| b == 0)
        .filter(|s| !s.is_empty())
        .filter_map(|s| std::str::from_utf8(s).ok())
        .map(|s| s.to_string())
        .collect()
}

/// `<identifier>\x1f<replacement>`: the file path and the replacement can contain ':', neither
/// contains the unit separator
fn parse_encoding(diag: &MullDiagnostics, encoding: &str) -> Option<Mutant> {
    let Some((identifier, replacement)) = encoding
        .split_once('\x1f')
        .filter(|(identifier, _)| identifier.matches(':').count() >= 5)
    else {
        diag_warning!(
            diag,
            "Invalid mutant encoding (expected <mutator>:<file>:<line>:<column>:<line>:<column>\\x1f<replacement>): {}",
            encoding
        );
        return None;
    };

    Some(Mutant::from_identifier(
        diag,
        identifier.to_string(),
        replacement.into(),
    ))
}

pub fn extract_mutants<P: AsRef<Path>>(diag: &MullDiagnostics, paths: &[P]) -> Vec<Mutant> {
    let mut encodings = HashSet::new();

    for path in paths {
        let path_str = path.as_ref().to_string_lossy();
        let extracted = extract_from_file(diag, &path_str);
        diag_debug!(
            diag,
            "Extracted {} encoding(s) from '{}'",
            extracted.len(),
            path_str
        );
        encodings.extend(extracted);
    }

    let mut mutants: Vec<Mutant> = encodings
        .into_iter()
        .filter_map(|e| parse_encoding(diag, &e))
        .collect();

    mutants.sort();
    diag_debug!(diag, "Found {} unique mutant(s)", mutants.len());

    mutants
}

#[cfg(test)]
mod tests {
    use super::*;

    fn diag() -> MullDiagnostics {
        MullDiagnostics::with_writer(Box::new(std::io::sink()), false)
    }

    #[test]
    fn splits_at_the_separator() {
        let mutant = parse_encoding(
            &diag(),
            "cxx_remove_void_call:/a:b/c.cpp:7:3:7:20\x1f(void)(ns::advance())",
        )
        .unwrap();
        assert_eq!(
            mutant.identifier,
            "cxx_remove_void_call:/a:b/c.cpp:7:3:7:20"
        );
        assert_eq!(mutant.location.filepath, "/a:b/c.cpp");
        assert_eq!(mutant.location.end_column, 20);
        assert_eq!(mutant.replacement, "(void)(ns::advance())");
    }

    #[test]
    fn keeps_tabs_in_the_replacement() {
        let mutant = parse_encoding(
            &diag(),
            "cxx_remove_void_call:/c.cpp:7:3:8:20\x1f(void)(f(a,\n\tb))",
        )
        .unwrap();
        assert_eq!(mutant.replacement, "(void)(f(a,\n\tb))");
    }

    #[test]
    fn takes_an_empty_replacement() {
        let mutant = parse_encoding(&diag(), "cxx_remove_void_call:/c.cpp:7:3:7:20\x1f").unwrap();
        assert_eq!(mutant.replacement, "");
    }

    #[test]
    fn rejects_a_missing_separator() {
        assert!(parse_encoding(&diag(), "cxx_add_to_sub:/c.cpp:1:2:1:3:-").is_none());
    }
}
