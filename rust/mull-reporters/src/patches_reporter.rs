use crate::source_manager::SourceManager;
use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_debug, diag_info, diag_warning};
use mull_state::{ExecutionState, ExecutionStatus, Mutant};
use std::fs::{self, File};
use std::hash::{Hash, Hasher};
use std::io::Write;
use std::time::{SystemTime, UNIX_EPOCH};

pub struct PatchesReporter {
    patches_path: String,
    base_path_prefix: String,
}

impl PatchesReporter {
    pub fn new(report_dir: &str, report_name: &str, base_path: &str) -> Self {
        let dir = if report_dir.is_empty() {
            "."
        } else {
            report_dir
        };
        let name = if report_name.is_empty() {
            let ts = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|d| d.as_secs())
                .unwrap_or(0);
            ts.to_string()
        } else {
            report_name.to_string()
        };

        let patches_path = format!("{}/{}-patches", dir, name);
        let _ = fs::create_dir_all(&patches_path);

        Self {
            patches_path,
            base_path_prefix: if base_path.is_empty() {
                ".".to_string()
            } else {
                base_path.to_string()
            },
        }
    }
}

fn status_prefix(status: ExecutionStatus) -> &'static str {
    match status {
        ExecutionStatus::Passed => "survived-",
        ExecutionStatus::NotCovered => "uncovered-",
        _ => "killed-",
    }
}

fn sanitize_path(filepath: &str, base_path: &str) -> String {
    filepath
        .strip_prefix(base_path)
        .unwrap_or(filepath)
        .trim_start_matches('/')
        .replace(['/', '.'], "_")
}

const NAME_MAX: usize = 255;

fn patch_filename(
    patches_path: &str,
    base_path: &str,
    mutant: &Mutant,
    status: ExecutionStatus,
) -> String {
    let prefix = status_prefix(status);
    let loc = &mutant.location;
    let sanitized = sanitize_path(&loc.filepath, base_path);

    // Try full filename with path
    let full = format!(
        "{}-{}-L{}-C{}.patch",
        sanitized, mutant.mutator, loc.begin_line, loc.begin_column
    );
    if prefix.len() + full.len() < NAME_MAX {
        return format!("{}/{}{}", patches_path, prefix, full);
    }

    // Try without path
    let short = format!(
        "{}-L{}-C{}.patch",
        mutant.mutator, loc.begin_line, loc.begin_column
    );
    if prefix.len() + short.len() < NAME_MAX {
        return format!("{}/{}{}", patches_path, prefix, short);
    }

    // Fall back to hash
    let mut hasher = std::hash::DefaultHasher::new();
    sanitized.hash(&mut hasher);
    mutant.mutator.hash(&mut hasher);
    let hash = hasher.finish();
    format!(
        "{}/{}{:016x}-L{}-C{}.patch",
        patches_path, prefix, hash, loc.begin_line, loc.begin_column
    )
}

fn strip_base_path<'a>(filepath: &'a str, base_path: &str) -> &'a str {
    filepath.strip_prefix(base_path).unwrap_or(filepath)
}

fn ensure_leading_slash(path: &str) -> String {
    if path.starts_with('/') {
        path.to_string()
    } else {
        format!("/{}", path)
    }
}

impl Reporter for PatchesReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());
        let mut source_manager = SourceManager::default();

        for (mutant, result) in &state.execution_results {
            let loc = &mutant.location;

            let Some(lines) = source_manager.get_source_lines(diag, loc) else {
                diag_warning!(
                    diag,
                    "PatchesReporter: Cannot report '{}': cannot read {}",
                    mutant.identifier,
                    loc.filepath
                );
                continue;
            };

            let filename = patch_filename(
                &self.patches_path,
                &self.base_path_prefix,
                mutant,
                result.status,
            );
            diag_debug!(diag, "Writing Patchfile: {}", filename);

            let source_path = strip_base_path(&loc.filepath, &self.base_path_prefix);
            let source_path = ensure_leading_slash(source_path);
            let line_count = loc.end_line - loc.begin_line + 1;

            let mut file = match File::create(&filename) {
                Ok(f) => f,
                Err(err) => {
                    diag_warning!(diag, "Cannot create patchfile {}: {}", filename, err);
                    continue;
                }
            };

            // Write unified diff header
            let _ = writeln!(file, "--- a{} 0", source_path);
            let _ = writeln!(file, "+++ b{} 0", source_path);
            let _ = writeln!(
                file,
                "@@ -{},{} +{},1 @@",
                loc.begin_line, line_count, loc.begin_line
            );

            // Write removed lines
            for line in &lines {
                let _ = writeln!(file, "-{}", line);
            }

            // Write replacement line
            let first_line = lines.first().map(|s| s.as_str()).unwrap_or("");
            let last_line = lines.last().map(|s| s.as_str()).unwrap_or("");
            let col_start = (loc.begin_column as usize).saturating_sub(1);
            let col_end = (loc.end_column as usize).saturating_sub(1);

            let prefix: String = first_line.chars().take(col_start).collect();
            let suffix: String = last_line.chars().skip(col_end).collect();
            let _ = writeln!(file, "+{}{}{}", prefix, mutant.replacement, suffix);

            // Write info footer
            let mut info: Vec<_> = state.info.iter().collect();
            info.sort_by_key(|(k, _)| *k);
            let _ = writeln!(file, "--");
            for (key, value) in info {
                let _ = writeln!(file, "{}: {}", key, value);
            }
        }

        diag_info!(diag, "Patchfiles can be found at '{}'", self.patches_path);
    }
}
