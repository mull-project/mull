use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_error, diag_info, diag_warning};
use mull_state::{ExecutionState, ExecutionStatus};
use serde::Serialize;
use std::collections::BTreeMap;
use std::fs::{self, File};
use std::io::{BufWriter, Write};
use std::time::{SystemTime, UNIX_EPOCH};

/// Mutation Testing Elements schema version 1.7
/// https://github.com/stryker-mutator/mutation-testing-elements

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct MutationTestingReport {
    #[serde(skip_serializing_if = "Option::is_none")]
    config: Option<BTreeMap<String, String>>,
    files: BTreeMap<String, FileResult>,
    thresholds: Thresholds,
    schema_version: &'static str,
    #[serde(skip_serializing_if = "Option::is_none")]
    framework: Option<Framework>,
    #[serde(skip_serializing_if = "Option::is_none")]
    mutation_score: Option<u32>,
}

#[derive(Serialize)]
struct Thresholds {
    high: u32,
    low: u32,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct FileResult {
    language: &'static str,
    mutants: Vec<MutantResult>,
    source: String,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct MutantResult {
    id: String,
    mutator_name: String,
    replacement: String,
    location: Location,
    status: MutantStatus,
}

#[derive(Serialize)]
struct Location {
    start: Position,
    end: Position,
}

#[derive(Serialize)]
struct Position {
    line: u64,
    column: u64,
}

#[derive(Serialize)]
#[allow(dead_code)]
enum MutantStatus {
    Killed,
    Survived,
    NoCoverage,
    Timeout,
    RuntimeError,
    CompileError,
    Ignored,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Framework {
    #[serde(skip_serializing_if = "Option::is_none")]
    branding_information: Option<BrandingInformation>,
    name: &'static str,
    #[serde(skip_serializing_if = "Option::is_none")]
    version: Option<String>,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct BrandingInformation {
    #[serde(skip_serializing_if = "Option::is_none")]
    homepage_url: Option<String>,
}

pub struct ElementsReporter {
    json_path: String,
    html_path: String,
    filename: String,
}

impl ElementsReporter {
    pub fn new(report_dir: &str, report_name: &str) -> Self {
        let dir = if report_dir.is_empty() {
            "."
        } else {
            report_dir
        };
        let _ = fs::create_dir_all(dir);

        let filename = if report_name.is_empty() {
            let ts = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|d| d.as_secs())
                .unwrap_or(0);
            ts.to_string()
        } else {
            report_name.to_string()
        };

        Self {
            json_path: format!("{}/{}.json", dir, filename),
            html_path: format!("{}/{}.html", dir, filename),
            filename,
        }
    }

    fn generate_html(&self, diag: &MullDiagnostics) {
        let html = format!(
            r#"<!DOCTYPE html>
<head>
  <title>Mutation Testing Elements</title>
  <script defer src="https://www.unpkg.com/mutation-testing-elements"></script>
</head>
<body>
  <mutation-test-report-app src="{}.json"></mutation-test-report-app>
</body>
</html>"#,
            self.filename
        );

        match File::create(&self.html_path) {
            Ok(mut f) => {
                let _ = f.write_all(html.as_bytes());
            }
            Err(err) => {
                diag_warning!(
                    diag,
                    "Cannot create HTML report {}: {}",
                    self.html_path,
                    err
                );
            }
        }
    }
}

fn execution_status_to_mutant_status(status: ExecutionStatus) -> MutantStatus {
    match status {
        ExecutionStatus::Passed => MutantStatus::Survived,
        ExecutionStatus::NotCovered => MutantStatus::NoCoverage,
        ExecutionStatus::Timedout => MutantStatus::Timeout,
        ExecutionStatus::Crashed | ExecutionStatus::AbnormalExit => MutantStatus::RuntimeError,
        _ => MutantStatus::Killed,
    }
}

impl Reporter for ElementsReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());

        self.generate_html(diag);

        // Group mutants by file
        let mut files: BTreeMap<String, FileResult> = BTreeMap::new();
        let mut killed_count = 0usize;

        for (mutant, result) in &state.execution_results {
            let filepath = &mutant.location.filepath;

            if !files.contains_key(filepath) {
                let source = fs::read_to_string(filepath).unwrap_or_else(|err| {
                    diag_warning!(
                        diag,
                        "ElementsReporter: Cannot report '{}': cannot read {}: {}",
                        mutant.identifier,
                        mutant.location.filepath,
                        err
                    );
                    String::new()
                });
                files.insert(
                    filepath.clone(),
                    FileResult {
                        language: "cpp",
                        source,
                        mutants: Vec::new(),
                    },
                );
            }

            let status = execution_status_to_mutant_status(result.status);
            if matches!(status, MutantStatus::Killed) {
                killed_count += 1;
            }

            let mutant_result = MutantResult {
                id: mutant.identifier.clone(),
                mutator_name: mutant.mutator.clone(),
                replacement: mutant.replacement.clone(),
                location: Location {
                    start: Position {
                        line: mutant.location.begin_line,
                        column: mutant.location.begin_column,
                    },
                    end: Position {
                        line: mutant.location.end_line,
                        column: mutant.location.end_column,
                    },
                },
                status,
            };

            files.get_mut(filepath).unwrap().mutants.push(mutant_result);
        }

        let total = state.execution_results.len();
        let score = (killed_count * 100) / total;

        let version = state.info.get("Mull Version").map(|v| {
            let llvm = state
                .info
                .get("LLVM Version")
                .map(|s| s.as_str())
                .unwrap_or("");
            format!("{}, LLVM {}", v, llvm)
        });

        let homepage_url = state.info.get("URL").cloned();

        let report = MutationTestingReport {
            schema_version: "1.7",
            thresholds: Thresholds { high: 80, low: 60 },
            files,
            mutation_score: Some(score as u32),
            config: Some(
                state
                    .info
                    .iter()
                    .map(|(k, v)| (k.clone(), v.clone()))
                    .collect(),
            ),
            framework: Some(Framework {
                name: "Mull",
                version,
                branding_information: Some(BrandingInformation { homepage_url }),
            }),
        };

        match File::create(&self.json_path) {
            Ok(file) => {
                if let Err(err) = serde_json::to_writer(BufWriter::new(file), &report) {
                    diag_error!(diag, "Cannot write JSON report: {}", err);
                }
                diag_info!(
                    diag,
                    "Mutation Testing Elements reporter: generated report to {}",
                    self.json_path
                );
            }
            Err(err) => {
                diag_error!(
                    diag,
                    "Cannot create JSON report {}: {}",
                    self.json_path,
                    err
                );
            }
        }
    }
}
