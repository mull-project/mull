use crate::source_manager::SourceManager;
use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_error, diag_info, diag_raw, diag_warning};
use mull_state::{ExecutionState, ExecutionStatus, Mutant};
use std::fs::File;
use std::io::BufWriter;

pub struct IDEReporter {
    show_killed: bool,
    report_filepath: Option<String>,
}

impl IDEReporter {
    pub fn new(show_killed: bool, report_dir: &str, report_name: &str) -> Self {
        Self {
            show_killed,
            report_filepath: if report_name.is_empty() {
                None
            } else {
                Some(format!("{}/{}.txt", report_dir, report_name))
            },
        }
    }
}

fn print_mutants(
    diag: &MullDiagnostics,
    source_manager: &mut SourceManager,
    mut mutants: Vec<&Mutant>,
    total_size: usize,
    label: &str,
) {
    if mutants.is_empty() {
        return;
    }
    mutants.sort();
    diag_info!(
        diag,
        "{} mutants ({}/{}):",
        label,
        mutants.len(),
        total_size
    );

    for mutant in mutants {
        let loc = &mutant.location;
        let message = match source_manager.get_source(diag, loc) {
            Some(original) => format!("Replaced {} with {}", original, mutant.replacement),
            None => format!("Replaced with {}", mutant.replacement),
        };
        diag_raw!(
            diag,
            "{}:{}:{}: warning: {}: {} [{}]",
            loc.filepath,
            loc.begin_line,
            loc.begin_column,
            label,
            message,
            mutant.mutator
        );
        match source_manager.get_source_line_with_caret(diag, loc) {
            Some(s) => diag_raw!(diag, "{}", s),
            None => diag_warning!(
                diag,
                "IDEReporter: Cannot report '{}': cannot read {}",
                mutant.identifier,
                mutant.location.filepath
            ),
        }
    }
}

fn report_internal(diag: &MullDiagnostics, state: &ExecutionState, show_killed: bool) {
    assert!(!state.execution_results.is_empty());

    let mut survived = Vec::new();
    let mut not_covered = Vec::new();
    let mut killed = Vec::new();

    for (mutant, result) in &state.execution_results {
        match result.status {
            ExecutionStatus::Passed => survived.push(mutant),
            ExecutionStatus::NotCovered => not_covered.push(mutant),
            _ => killed.push(mutant),
        }
    }

    let total = state.execution_results.len();
    let score = killed.len() * 100 / total;
    let all_killed = survived.is_empty() && not_covered.is_empty();
    let mut source_manager = SourceManager::default();

    if show_killed {
        print_mutants(diag, &mut source_manager, killed, total, "Killed");
    }
    print_mutants(diag, &mut source_manager, survived, total, "Survived");
    print_mutants(diag, &mut source_manager, not_covered, total, "Not covered");

    diag_info!(diag, "Mutation score: {}%", score);
    if all_killed {
        diag_info!(diag, "All mutations have been killed");
    }
}

impl Reporter for IDEReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        let Some(path) = &self.report_filepath else {
            report_internal(diag, state, self.show_killed);
            return;
        };

        match File::create(path) {
            Ok(file) => {
                let file_diag = MullDiagnostics::with_writer(Box::new(BufWriter::new(file)), false);
                report_internal(&file_diag, state, self.show_killed);
                diag_info!(diag, "IDE Report saved into {}", path);
            }
            Err(err) => diag_error!(diag, "Cannot create report file {}: {}", path, err),
        }
    }
}
