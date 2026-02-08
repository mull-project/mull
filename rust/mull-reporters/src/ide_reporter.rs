use crate::Reporter;
use mull_state::ExecutionState;

pub struct IDEReporter {}

impl IDEReporter {
    pub fn new() -> Self {
        Self {}
    }
}

impl Reporter for IDEReporter {
    fn report(&self, state: &ExecutionState) {
        for (mutant, result) in &state.execution_results {
            println!(
                "{}: {} (exit code: {}, {}ms)",
                mutant.identifier, result.status, result.exit_status, result.running_time
            );
        }
    }
}
