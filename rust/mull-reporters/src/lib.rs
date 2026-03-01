mod elements_reporter;
mod github_annotations_reporter;
mod ide_reporter;
mod patches_reporter;
mod source_manager;
mod sqlite_reporter;

use mull_core::diagnostics::MullDiagnostics;
use mull_state::ExecutionState;

pub use elements_reporter::ElementsReporter;
pub use github_annotations_reporter::GitHubAnnotationsReporter;
pub use ide_reporter::IDEReporter;
pub use mull_core::config::ReporterKind;
pub use patches_reporter::PatchesReporter;
pub use sqlite_reporter::SQLiteReporter;

pub trait Reporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState);
}

pub struct ReporterConfig {
    pub reporters: Vec<ReporterKind>,
    pub report_dir: String,
    pub report_name: String,
    pub report_patch_base: String,
    pub ide_reporter_show_killed: bool,
}

pub fn create_reporters(config: &ReporterConfig) -> Vec<Box<dyn Reporter>> {
    let reporters: Vec<Box<dyn Reporter>> = config
        .reporters
        .iter()
        .map(|kind| -> Box<dyn Reporter> {
            match kind {
                ReporterKind::IDE => Box::new(IDEReporter::new(
                    config.ide_reporter_show_killed,
                    &config.report_dir,
                    &config.report_name,
                )),
                ReporterKind::SQLite => {
                    Box::new(SQLiteReporter::new(&config.report_dir, &config.report_name))
                }
                ReporterKind::GitHubAnnotations => Box::new(GitHubAnnotationsReporter::new()),
                ReporterKind::Patches => Box::new(PatchesReporter::new(
                    &config.report_dir,
                    &config.report_name,
                    &config.report_patch_base,
                )),
                ReporterKind::Elements => Box::new(ElementsReporter::new(
                    &config.report_dir,
                    &config.report_name,
                )),
            }
        })
        .collect();

    if reporters.is_empty() {
        vec![Box::new(IDEReporter::new(
            config.ide_reporter_show_killed,
            &config.report_dir,
            &config.report_name,
        ))]
    } else {
        reporters
    }
}
