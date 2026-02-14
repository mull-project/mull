use std::time::Instant;

use mull_core::{config::SharedCli, diag_info, utils::format_elapsed, MullDiagnostics};
use mull_reporters::{create_reporters, ReporterConfig};
use mull_state::{ExecutionState, ExecutionStatus};

fn report_mutation_score(
    diag: &MullDiagnostics,
    state: &ExecutionState,
    allow_survived: bool,
    threshold: u32,
) -> bool {
    assert!(!state.execution_results.is_empty());
    let mut killed = 0;
    let mut survived = 0;
    for (_, result) in &state.execution_results {
        if result.status != ExecutionStatus::Passed && result.status != ExecutionStatus::NotCovered
        {
            killed += 1;
        }
        if result.status == ExecutionStatus::Passed {
            survived += 1;
        }
    }

    let total = state.execution_results.len();
    let score = killed * 100 / total;
    // diag_info!(diag, "Mutation score: {}%", score)
    if survived != 0 {
        diag_info!(diag, "Surviving mutants: {}", survived);
        if !allow_survived && threshold > score as u32 {
            return true;
        }
    }

    false
}

pub fn show_report(
    diag: &MullDiagnostics,
    exec_state: &ExecutionState,
    cli: SharedCli,
    start: &Instant,
    label: &str,
) {
    let should_fail = if exec_state.execution_results.is_empty() {
        diag_info!(diag, "No mutants found. Mutation score: infinitely high");
        false
    } else {
        let reporter_config = ReporterConfig {
            reporters: cli.reporters,
            report_dir: cli.report_dir,
            report_name: cli.report_name.unwrap_or_default(),
            ide_reporter_show_killed: cli.ide_reporter_show_killed,
            report_patch_base: cli.report_patch_base,
        };
        let reporters = create_reporters(&reporter_config);
        for reporter in &reporters {
            reporter.report(&diag, &exec_state);
        }
        report_mutation_score(
            &diag,
            &exec_state,
            cli.allow_surviving,
            cli.mutation_score_threshold,
        )
    };

    diag_info!(
        diag,
        "Total {} time: {}",
        label,
        format_elapsed(start.elapsed())
    );
    if should_fail {
        std::process::exit(1);
    }
}
