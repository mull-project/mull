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

fn parse_encoding(diag: &MullDiagnostics, encoding: &str) -> Option<Mutant> {
    let chunks: Vec<&str> = encoding.split(':').collect();

    if chunks.len() < 7 {
        diag_warning!(
            diag,
            "Invalid mutant encoding (expected at least 7 parts): {}",
            encoding
        );
        return None;
    }

    let replacement = chunks[6];
    let identifier = encoding[..encoding.rfind(':').unwrap()].to_string();

    Some(Mutant::from_identifier(
        diag,
        identifier,
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
