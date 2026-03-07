use mull_state::{ExecutionResult, ExecutionStatus};
use wait_timeout::ChildExt;

use std::io::Read;
use std::process::{Command, Stdio};
use std::time::{Duration, Instant};

pub fn run_program(
    executable: &str,
    args: &[String],
    env: &[(&str, &str)],
    timeout: Duration,
    capture_output: bool,
) -> ExecutionResult {
    let start = Instant::now();

    let mut cmd = Command::new(executable);
    cmd.args(args);
    for (k, v) in env {
        cmd.env(k, v);
    }

    if capture_output {
        cmd.stdout(Stdio::piped());
        cmd.stderr(Stdio::piped());
    } else {
        cmd.stdout(Stdio::null());
        cmd.stderr(Stdio::null());
    }

    let mut child = match cmd.spawn() {
        Ok(c) => c,
        Err(e) => {
            return ExecutionResult {
                status: ExecutionStatus::Failed,
                exit_status: -2, // TODO: triggering sqlite rebuild, revert
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: String::new(),
                stderr_output: format!("Failed to spawn: {}", e),
            };
        }
    };

    // Capture whatever output is available (None if Stdio::null was used)
    let mut stdout = String::new();
    let mut stderr = String::new();
    let capture = |child: &mut std::process::Child, stdout: &mut String, stderr: &mut String| {
        if let Some(mut out) = child.stdout.take() {
            let _ = out.read_to_string(stdout);
        }
        if let Some(mut err) = child.stderr.take() {
            let _ = err.read_to_string(stderr);
        }
    };

    match child.wait_timeout(timeout) {
        Ok(Some(status)) => {
            capture(&mut child, &mut stdout, &mut stderr);
            ExecutionResult {
                status: if status.success() {
                    ExecutionStatus::Passed
                } else {
                    ExecutionStatus::Failed
                },
                exit_status: status.code().unwrap_or(-1),
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: stdout,
                stderr_output: stderr,
            }
        }
        Ok(None) => {
            // Timed out - kill first, then capture whatever was buffered
            let _ = child.kill();
            let _ = child.wait();
            capture(&mut child, &mut stdout, &mut stderr);
            ExecutionResult {
                status: ExecutionStatus::Timedout,
                exit_status: -1,
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: stdout,
                stderr_output: stderr,
            }
        }
        Err(e) => ExecutionResult {
            status: ExecutionStatus::Failed,
            exit_status: -1,
            running_time: start.elapsed().as_millis() as i64,
            stdout_output: String::new(),
            stderr_output: format!("Wait error: {}", e),
        },
    }
}
