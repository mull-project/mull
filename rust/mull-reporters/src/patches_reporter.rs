use crate::source_manager::{column_offset, SourceManager};
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

/// Unified diff of a single mutant, paths relative to `base_path`
pub(crate) fn render_patch(
    diag: &MullDiagnostics,
    source_manager: &mut SourceManager,
    mutant: &Mutant,
    base_path: &str,
) -> Result<String, String> {
    let loc = &mutant.location;
    let lines = source_manager
        .get_source_lines(diag, loc)
        .ok_or_else(|| format!("cannot read {}", loc.filepath))?;

    let first_line = lines.first().map(|s| s.as_str()).unwrap_or("");
    let last_line = lines.last().map(|s| s.as_str()).unwrap_or("");
    let (Some(prefix), Some(suffix)) = (
        column_offset(first_line, loc.begin_column).map(|offset| &first_line[..offset]),
        column_offset(last_line, loc.end_column).map(|offset| &last_line[offset..]),
    ) else {
        return Err(format!("invalid columns in {}", loc.filepath));
    };

    let source_path = ensure_leading_slash(strip_base_path(&loc.filepath, base_path));
    let line_count = loc.end_line - loc.begin_line + 1;
    // A replacement keeping the arguments of a call can span several lines
    let mutated = format!("{}{}{}", prefix, mutant.replacement, suffix);
    let mutated_lines: Vec<&str> = mutated.split('\n').collect();
    let mut patch = format!(
        "--- a{0}\n+++ b{0}\n@@ -{1},{2} +{1},{3} @@\n",
        source_path,
        loc.begin_line,
        line_count,
        mutated_lines.len()
    );
    for line in &lines {
        patch += &format!("-{}\n", line);
    }
    for line in &mutated_lines {
        patch += &format!("+{}\n", line);
    }
    Ok(patch)
}

impl Reporter for PatchesReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());
        let mut source_manager = SourceManager::default();

        for (mutant, result) in &state.execution_results {
            let patch =
                match render_patch(diag, &mut source_manager, mutant, &self.base_path_prefix) {
                    Ok(patch) => patch,
                    Err(reason) => {
                        diag_warning!(
                            diag,
                            "PatchesReporter: Cannot report '{}': {}",
                            mutant.identifier,
                            reason
                        );
                        continue;
                    }
                };

            let filename = patch_filename(
                &self.patches_path,
                &self.base_path_prefix,
                mutant,
                result.status,
            );
            diag_debug!(diag, "Writing Patchfile: {}", filename);

            let mut file = match File::create(&filename) {
                Ok(f) => f,
                Err(err) => {
                    diag_warning!(diag, "Cannot create patchfile {}: {}", filename, err);
                    continue;
                }
            };
            let _ = file.write_all(patch.as_bytes());

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

#[cfg(test)]
mod tests {
    use super::*;

    fn patch_of(source: &str, identifier: &str, replacement: &str) -> String {
        let dir = std::env::temp_dir().join(format!("mull-patch-{}", std::process::id()));
        std::fs::create_dir_all(&dir).unwrap();
        let path = dir.join(format!("{}.cpp", replacement.len()));
        std::fs::write(&path, source).unwrap();
        let diag = MullDiagnostics::with_writer(Box::new(std::io::sink()), false);
        let identifier = identifier.replace("FILE", path.to_str().unwrap());
        let mutant = Mutant::from_identifier(&diag, identifier, replacement.to_string());
        let mut source_manager = SourceManager::default();
        let patch = render_patch(&diag, &mut source_manager, &mutant, dir.to_str().unwrap());
        patch
            .unwrap()
            .lines()
            .skip(2)
            .collect::<Vec<_>>()
            .join("\n")
    }

    #[test]
    fn keeps_call_operands() {
        let patch = patch_of(
            "  ignore(advance());\n",
            "cxx_remove_void_call:FILE:1:3:1:20",
            "(void)(advance())",
        );
        assert_eq!(
            patch,
            "@@ -1,1 +1,1 @@\n-  ignore(advance());\n+  (void)(advance());"
        );
    }

    #[test]
    fn splits_a_multiline_replacement() {
        let patch = patch_of(
            "  ignore(compute(1,\n                 2));\n",
            "cxx_remove_void_call:FILE:1:3:2:21",
            "(void)(compute(1,\n                 2))",
        );
        assert_eq!(
            patch,
            "@@ -1,2 +1,2 @@\n-  ignore(compute(1,\n-                 2));\n+  (void)(compute(1,\n+                 2));"
        );
    }
}
