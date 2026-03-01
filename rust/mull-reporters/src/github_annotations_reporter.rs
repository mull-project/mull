use crate::source_manager::SourceManager;
use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_info, diag_raw};
use mull_state::{ExecutionState, ExecutionStatus};

pub struct GitHubAnnotationsReporter {}

impl GitHubAnnotationsReporter {
    pub fn new() -> Self {
        Self {}
    }
}

impl Reporter for GitHubAnnotationsReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());
        let mut source_manager = SourceManager::default();
        let mut results: Vec<_> = state.execution_results.iter().collect();
        results.sort_by_key(|(mutant, _)| *mutant);
        // TODO: GitHub
        diag_info!(diag, "Github Annotations:");
        for (mutant, result) in results {
            if result.status != ExecutionStatus::Passed {
                continue;
            }
            let loc = &mutant.location;
            let original = source_manager.get_source(diag, loc);
            let message = if let Some(original) = original {
                format!("Replaced {} with {}", original, mutant.replacement)
            } else {
                format!("Replaced with {}", mutant.replacement)
            };

            // TODO: Add a title parameter instead of mutator in the brackets
            diag_raw!(
                diag,
                "::warning file={},line={},col={},endLine={},endColumn={}::[{}] {}",
                loc.filepath,
                loc.begin_line,
                loc.begin_column,
                loc.end_line,
                loc.end_column,
                mutant.mutator,
                message
            );
        }
    }
}
