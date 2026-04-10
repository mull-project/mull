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
                exit_status: -1,
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: String::new(),
                stderr_output: format!("Failed to spawn: {}", e),
            };
        }
    };

    // Drain stdout/stderr in background threads to prevent pipe deadlock.
    // If the child writes enough to fill the OS pipe buffer (~64KB) before
    // exiting, it will block on write() and wait_timeout will never return.
    let stdout_thread = child.stdout.take().map(|mut out| {
        std::thread::spawn(move || {
            let mut buf = String::new();
            let _ = out.read_to_string(&mut buf);
            buf
        })
    });
    let stderr_thread = child.stderr.take().map(|mut err| {
        std::thread::spawn(move || {
            let mut buf = String::new();
            let _ = err.read_to_string(&mut buf);
            buf
        })
    });

    let join_output = |stdout_thread: Option<std::thread::JoinHandle<String>>,
                       stderr_thread: Option<std::thread::JoinHandle<String>>|
     -> (String, String) {
        let stdout = stdout_thread
            .map(|t| t.join().unwrap_or_default())
            .unwrap_or_default();
        let stderr = stderr_thread
            .map(|t| t.join().unwrap_or_default())
            .unwrap_or_default();
        (stdout, stderr)
    };

    match child.wait_timeout(timeout) {
        Ok(Some(status)) => {
            let (stdout, stderr) = join_output(stdout_thread, stderr_thread);
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
            // Timed out - kill first, then join reader threads
            let _ = child.kill();
            let _ = child.wait();
            let (stdout, stderr) = join_output(stdout_thread, stderr_thread);
            ExecutionResult {
                status: ExecutionStatus::Timedout,
                exit_status: -1,
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: stdout,
                stderr_output: stderr,
            }
        }
        Err(e) => {
            let (stdout, stderr) = join_output(stdout_thread, stderr_thread);
            ExecutionResult {
                status: ExecutionStatus::Failed,
                exit_status: -1,
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: stdout,
                stderr_output: format!("Wait error: {}\n{}", e, stderr),
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Duration;

    /// Verify that run_program does not deadlock when a child process writes
    /// more output than the OS pipe buffer (typically ~64KB) before exiting.
    ///
    /// Without draining stdout/stderr in background threads before calling
    /// wait_timeout(), the child blocks on write() once the pipe buffer fills
    /// while the parent blocks on wait() — neither can proceed. This test
    /// would hang until the timeout fires on an unfixed implementation.
    #[test]
    fn large_stdout_does_not_deadlock() {
        // `seq 1 50000` emits ~290KB of text, well above the ~64KB pipe buffer.
        let result = run_program(
            "seq",
            &["1".to_string(), "50000".to_string()],
            &[],
            Duration::from_secs(10),
            true,
        );
        assert_ne!(
            result.status,
            ExecutionStatus::Timedout,
            "run_program timed out — likely a pipe deadlock: child blocked on \
             write() while parent blocked on wait()"
        );
        assert_eq!(result.status, ExecutionStatus::Passed);
        assert!(
            result.stdout_output.len() > 64 * 1024,
            "expected > 64KB of stdout, got {} bytes",
            result.stdout_output.len()
        );
    }
}
