use crate::ExecutionStatus;
use mull_core::{diag_error, MullDiagnostics};

#[derive(Debug)]
pub struct ExecutionResult {
    pub status: ExecutionStatus,
    pub exit_status: i32,
    pub running_time: i64,
    pub stdout_output: String,
    pub stderr_output: String,
}

impl ExecutionResult {
    pub fn error_on_failure(&self, diag: &MullDiagnostics, label: &str) {
        if self.status != ExecutionStatus::Passed {
            diag_error!(
                diag,
                "{}
status: {}
stdout: '{}'
stderr: '{}'",
                label,
                self.status,
                self.stdout_output,
                self.stderr_output
            );
        }
    }
}
