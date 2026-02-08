use mull_core::{diag_error, diagnostics::MullDiagnostics};

#[derive(Debug)]
pub struct SourceRange {
    pub filepath: String,
    pub begin_line: u64,
    pub begin_column: u64,
    pub end_line: u64,
    pub end_column: u64,
}

impl SourceRange {
    pub fn new(
        filepath: String,
        begin_line: u64,
        begin_column: u64,
        end_line: u64,
        end_column: u64,
    ) -> Self {
        Self {
            filepath,
            begin_line,
            begin_column,
            end_line,
            end_column,
        }
    }
}

pub struct Mutant {
    pub identifier: String,
    pub mutator: String,
    pub location: SourceRange,
    pub replacement: String,
}

fn parse_u64(diag: &MullDiagnostics, chunk: &str, label: &str) -> u64 {
    match chunk.parse::<u64>() {
        Ok(n) => n,
        Err(err) => {
            diag_error!(diag, "Cannot parse '{}' as {}: {}", chunk, label, err)
        }
    }
}

impl Mutant {
    pub fn from_identifier(
        diag: &MullDiagnostics,
        identifier: String,
        replacement: String,
    ) -> Self {
        let chunks = identifier.split(":").collect::<Vec<&str>>();
        assert_eq!(chunks.len(), 6);
        let mutator = chunks[0].to_string();
        let location = SourceRange {
            filepath: chunks[1].into(),
            begin_line: parse_u64(diag, chunks[2], "begin line"),
            begin_column: parse_u64(diag, chunks[3], "begin column"),
            end_line: parse_u64(diag, chunks[4], "end line"),
            end_column: parse_u64(diag, chunks[5], "end column"),
        };

        Self {
            identifier,
            mutator,
            location,
            replacement,
        }
    }
}

impl std::fmt::Debug for Mutant {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Mutant")
            .field("identifier", &self.identifier)
            .finish()
    }
}

impl std::hash::Hash for Mutant {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.identifier.hash(state);
    }
}

impl PartialEq for Mutant {
    fn eq(&self, other: &Self) -> bool {
        self.identifier == other.identifier
    }
}

impl Eq for Mutant {}

impl Ord for Mutant {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        (
            &self.location.filepath,
            self.location.begin_line,
            self.location.begin_column,
            &self.mutator,
        )
            .cmp(&(
                &other.location.filepath,
                other.location.begin_line,
                other.location.begin_column,
                &other.mutator,
            ))
    }
}

impl PartialOrd for Mutant {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
