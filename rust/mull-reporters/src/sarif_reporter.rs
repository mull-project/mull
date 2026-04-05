use crate::source_manager::SourceManager;
use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_error, diag_info};
use mull_state::{ExecutionState, ExecutionStatus};
use serde_sarif::sarif::{
    ArtifactLocation, Location, Message, MultiformatMessageString, PhysicalLocation, Region,
    ReportingDescriptor, Result as SarifResult, ResultLevel, Run, Sarif, Tool, ToolComponent,
};
use std::collections::BTreeMap;
use std::fs::File;
use std::io::BufWriter;
use std::time::{SystemTime, UNIX_EPOCH};

const SRCROOT: &str = "%SRCROOT%";

pub struct SarifReporter {
    output_path: String,
    base_path: String,
}

impl SarifReporter {
    pub fn new(report_dir: &str, report_name: &str, base_path: &str) -> Self {
        let dir = if report_dir.is_empty() {
            "."
        } else {
            report_dir
        };
        let _ = std::fs::create_dir_all(dir);

        let filename = if report_name.is_empty() {
            SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|d| d.as_secs())
                .unwrap_or(0)
                .to_string()
        } else {
            report_name.to_string()
        };

        Self {
            output_path: format!("{}/{}.sarif", dir, filename),
            base_path: base_path.to_string(),
        }
    }

    fn make_artifact_location(&self, filepath: &str) -> ArtifactLocation {
        if self.base_path.is_empty() {
            return ArtifactLocation::builder()
                .uri(filepath.to_string())
                .build();
        }

        let relative = filepath
            .strip_prefix(&self.base_path)
            .unwrap_or(filepath)
            .trim_start_matches('/');

        ArtifactLocation::builder()
            .uri(relative.to_string())
            .uri_base_id(SRCROOT.to_string())
            .build()
    }
}

fn mutation_description(original: Option<&str>, replacement: &str) -> String {
    match original {
        Some(orig) => format!("Replaces `{}` with `{}`", orig, replacement),
        None => format!("Replaces with `{}`", replacement),
    }
}

impl Reporter for SarifReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());

        let mut source_manager = SourceManager::default();

        let mut sorted: Vec<_> = state.execution_results.iter().collect();
        sorted.sort_by_key(|(mutant, _)| *mutant);

        // (mutator -> (replacement, original)) — first example seen per mutator, for rule descriptions
        let mut rule_examples: BTreeMap<String, (String, Option<String>)> = BTreeMap::new();
        let mut results: Vec<SarifResult> = Vec::new();
        for (mutant, result) in &sorted {
            let level = match result.status {
                ExecutionStatus::Passed => ResultLevel::Warning,
                ExecutionStatus::NotCovered => ResultLevel::Note,
                _ => continue,
            };

            let loc = &mutant.location;
            let original = source_manager.get_source(diag, loc);

            rule_examples
                .entry(mutant.mutator.clone())
                .or_insert_with(|| (mutant.replacement.clone(), original.clone()));

            let message_text = mutation_description(original.as_deref(), &mutant.replacement);

            let sarif_result = SarifResult::builder()
                .message(Message::builder().text(message_text).build())
                .rule_id(mutant.mutator.clone())
                .level(level)
                .partial_fingerprints(BTreeMap::from([(
                    "primaryLocationLineHash".to_string(),
                    mutant.identifier.clone(),
                )]))
                .locations(vec![Location::builder()
                    .physical_location(
                        PhysicalLocation::builder()
                            .artifact_location(self.make_artifact_location(&loc.filepath))
                            .region(
                                Region::builder()
                                    .start_line(loc.begin_line as i64)
                                    .start_column(loc.begin_column as i64)
                                    .end_line(loc.end_line as i64)
                                    .end_column(loc.end_column as i64)
                                    .build(),
                            )
                            .build(),
                    )
                    .build()])
                .build();

            results.push(sarif_result);
        }

        let rules: Vec<ReportingDescriptor> = rule_examples
            .into_iter()
            .map(|(id, (replacement, original))| {
                let desc = mutation_description(original.as_deref(), &replacement);
                ReportingDescriptor::builder()
                    .id(id)
                    .short_description(MultiformatMessageString::builder().text(desc).build())
                    .build()
            })
            .collect();

        let driver = ToolComponent::builder()
            .name("Mull".to_string())
            .version(state.info.get("Mull Version").cloned().unwrap_or_default())
            .information_uri(state.info.get("URL").cloned().unwrap_or_default())
            .rules(rules)
            .build();

        let tool = Tool::builder().driver(driver).build();
        let run = if !self.base_path.is_empty() {
            let srcroot = ArtifactLocation::builder()
                .uri(format!("file://{}/", self.base_path.trim_end_matches('/')))
                .build();
            Run::builder()
                .tool(tool)
                .results(results)
                .original_uri_base_ids(BTreeMap::from([(SRCROOT.to_string(), srcroot)]))
                .build()
        } else {
            Run::builder().tool(tool).results(results).build()
        };

        let log = Sarif::builder()
            .version("2.1.0".to_string())
            .schema("https://json.schemastore.org/sarif-2.1.0.json".to_string())
            .runs(vec![run])
            .build();

        match File::create(&self.output_path) {
            Ok(file) => {
                if let Err(err) = serde_json::to_writer(BufWriter::new(file), &log) {
                    diag_error!(diag, "Cannot write SARIF report: {}", err);
                }
                diag_info!(
                    diag,
                    "SARIF reporter: generated report to {}",
                    self.output_path
                );
            }
            Err(err) => {
                diag_error!(
                    diag,
                    "Cannot create SARIF report {}: {}",
                    self.output_path,
                    err
                );
            }
        }
    }
}
