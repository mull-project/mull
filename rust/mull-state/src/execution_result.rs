use crate::ExecutionStatus;

#[derive(Debug)]
pub struct ExecutionResult {
    pub status: ExecutionStatus,
    pub exit_status: i32,
    pub running_time: i64,
    pub stdout_output: String,
    pub stderr_output: String,
}

impl ExecutionResult {
    pub fn debug_description(&self) -> String {
        // TODO: This is straight from C++, we should do better at the call site!
        format!(
            "Original test failed\nstatus: {}\nstdout: '{}'\nstderr: '{}'\n",
            self.status, self.stdout_output, self.stderr_output
        )
    }
}
