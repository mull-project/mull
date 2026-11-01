use mull_state::{ExecutionResult, ExecutionStatus};
use wait_timeout::ChildExt;

use std::collections::BTreeSet;
use std::io::Read;
use std::os::unix::process::CommandExt;
use std::process::{Command, Stdio};
use std::sync::mpsc::{channel, Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

// How long to keep reading after the child is gone. A leftover grandchild can
// hold the pipes open for as long as it runs.
const OUTPUT_DRAIN_TIMEOUT: Duration = Duration::from_secs(1);

static PROCESS_GROUPS: Mutex<BTreeSet<i32>> = Mutex::new(BTreeSet::new());

fn kill_process_group(group: i32) {
    unsafe {
        libc::killpg(group, libc::SIGKILL);
    }
}

// Each program runs in its own process group, so ^C does not reach it.
pub fn kill_programs_on_interrupt() -> Result<(), ctrlc::Error> {
    ctrlc::set_handler(|| {
        let groups = PROCESS_GROUPS.lock().unwrap();
        for group in groups.iter() {
            kill_process_group(*group);
        }
        std::process::exit(130);
    })
}

struct OutputReader {
    buffer: Arc<Mutex<Vec<u8>>>,
    finished: Receiver<()>,
}

impl OutputReader {
    fn start(mut stream: impl Read + Send + 'static) -> Self {
        let buffer = Arc::new(Mutex::new(Vec::new()));
        let (done, finished): (Sender<()>, Receiver<()>) = channel();
        let output = buffer.clone();
        std::thread::spawn(move || {
            let _done = done;
            let mut chunk = [0; 8192];
            while let Ok(count @ 1..) = stream.read(&mut chunk) {
                output.lock().unwrap().extend_from_slice(&chunk[..count]);
            }
        });
        Self { buffer, finished }
    }

    fn collect(self, deadline: Instant) -> String {
        let _ = self
            .finished
            .recv_timeout(deadline.saturating_duration_since(Instant::now()));
        String::from_utf8_lossy(&self.buffer.lock().unwrap()).into_owned()
    }
}

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
    cmd.process_group(0);
    // A background process group is stopped when it reads from the terminal.
    cmd.stdin(Stdio::null());

    if capture_output {
        cmd.stdout(Stdio::piped());
        cmd.stderr(Stdio::piped());
    } else {
        cmd.stdout(Stdio::null());
        cmd.stderr(Stdio::null());
    }

    // Held across spawn so that the interrupt handler cannot miss the group.
    let mut groups = PROCESS_GROUPS.lock().unwrap();
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
    let group = child.id() as i32;
    groups.insert(group);
    drop(groups);

    // Drain stdout/stderr in background threads to prevent pipe deadlock.
    // If the child writes enough to fill the OS pipe buffer (~64KB) before
    // exiting, it will block on write() and wait_timeout will never return.
    let stdout_reader = child.stdout.take().map(OutputReader::start);
    let stderr_reader = child.stderr.take().map(OutputReader::start);

    let join_output = |stdout_reader: Option<OutputReader>,
                       stderr_reader: Option<OutputReader>|
     -> (String, String) {
        let deadline = Instant::now() + OUTPUT_DRAIN_TIMEOUT;
        let stdout = stdout_reader
            .map(|r| r.collect(deadline))
            .unwrap_or_default();
        let stderr = stderr_reader
            .map(|r| r.collect(deadline))
            .unwrap_or_default();
        (stdout, stderr)
    };

    let wait_result = child.wait_timeout(timeout);
    if !matches!(wait_result, Ok(None)) {
        PROCESS_GROUPS.lock().unwrap().remove(&group);
    }

    match wait_result {
        Ok(Some(status)) => {
            let (stdout, stderr) = join_output(stdout_reader, stderr_reader);
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
            // Timed out - kill the group before reaping, so its ID stays valid
            kill_process_group(group);
            let _ = child.kill();
            let _ = child.wait();
            PROCESS_GROUPS.lock().unwrap().remove(&group);
            let (stdout, stderr) = join_output(stdout_reader, stderr_reader);
            ExecutionResult {
                status: ExecutionStatus::Timedout,
                exit_status: -1,
                running_time: start.elapsed().as_millis() as i64,
                stdout_output: stdout,
                stderr_output: stderr,
            }
        }
        Err(e) => {
            let (stdout, stderr) = join_output(stdout_reader, stderr_reader);
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

    fn is_running(pid: &str) -> bool {
        Command::new("kill")
            .args(["-0", pid])
            .stderr(Stdio::null())
            .status()
            .is_ok_and(|status| status.success())
    }

    /// A wrapper script is the direct child, so its grandchild holds the pipes
    /// open after the wrapper is killed.
    #[test]
    fn timeout_kills_grandchildren() {
        let result = run_program(
            "sh",
            &["-c".to_string(), "sleep 30 & echo $!; wait".to_string()],
            &[],
            Duration::from_millis(200),
            true,
        );
        assert_eq!(result.status, ExecutionStatus::Timedout);
        assert!(result.running_time < 5000, "took {}ms", result.running_time);

        let grandchild = result.stdout_output.trim();
        assert!(!grandchild.is_empty(), "partial output must be kept");
        let deadline = Instant::now() + Duration::from_secs(5);
        while is_running(grandchild) && Instant::now() < deadline {
            std::thread::sleep(Duration::from_millis(10));
        }
        assert!(!is_running(grandchild), "grandchild is still running");
    }

    /// A process left behind by a passing test must not hold up the runner.
    #[test]
    fn background_process_does_not_block_normal_exit() {
        let result = run_program(
            "sh",
            &["-c".to_string(), "sleep 10 & echo done".to_string()],
            &[],
            Duration::from_secs(30),
            true,
        );
        assert_eq!(result.status, ExecutionStatus::Passed);
        assert_eq!(result.stdout_output.trim(), "done");
        assert!(result.running_time < 5000, "took {}ms", result.running_time);
    }
}
