pub mod ide_reporter;

use mull_state::ExecutionState;

pub trait Reporter {
    fn report(&self, state: &ExecutionState);
}
