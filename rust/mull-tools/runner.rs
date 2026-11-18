use mull_core::{diag_error, MullDiagnostics};
use mull_state::{ExecutionResult, ExecutionStatus};
use wait_timeout::ChildExt;

use std::collections::BTreeSet;
use std::fmt;
use std::io::{self, Read};
use std::os::unix::process::CommandExt;
use std::process::{Command, Stdio};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::mpsc::{channel, Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

// How long to keep reading after the child is gone. A leftover grandchild can
// hold the pipes open for as long as it runs.
const OUTPUT_DRAIN_TIMEOUT: Duration = Duration::from_secs(1);

static PROCESS_GROUPS: Mutex<BTreeSet<i32>> = Mutex::new(BTreeSet::new());
static INTERRUPTED: AtomicBool = AtomicBool::new(false);

#[derive(Debug)]
pub enum RunError {
    Spawn(io::Error),
    Wait(io::Error),
    // The program was killed by the interrupt handler, its result is meaningless
    Interrupted,
}

impl fmt::Display for RunError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RunError::Spawn(e) => write!(f, "cannot spawn: {}", e),
            RunError::Wait(e) => write!(f, "cannot wait: {}", e),
            RunError::Interrupted => write!(f, "interrupted"),
        }
    }
}

impl RunError {
    pub fn exit(self, diag: &MullDiagnostics, executable: &str) -> ! {
        if let RunError::Interrupted = self {
            // The interrupt handler exits the process
            loop {
                std::thread::park();
            }
        }
        diag_error!(diag, "Cannot run '{}': {}", executable, self);
    }
}

fn kill_process_group(group: i32) {
    unsafe {
        libc::killpg(group, libc::SIGKILL);
    }
}

// Each program runs in its own process group, so ^C does not reach it.
pub fn kill_programs_on_interrupt() -> Result<(), ctrlc::Error> {
    ctrlc::set_handler(|| {
        INTERRUPTED.store(true, Ordering::SeqCst);
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
) -> Result<ExecutionResult, RunError> {
    run(executable, args, env, timeout, capture_output, &INTERRUPTED)
}

fn run(
    executable: &str,
    args: &[String],
    env: &[(&str, &str)],
    timeout: Duration,
    capture_output: bool,
    interrupted: &AtomicBool,
) -> Result<ExecutionResult, RunError> {
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
    if interrupted.load(Ordering::SeqCst) {
        return Err(RunError::Interrupted);
    }
    let mut child = cmd.spawn().map_err(RunError::Spawn)?;
    let group = child.id() as i32;
    groups.insert(group);
    drop(groups);

    // Drain stdout/stderr in background threads to prevent pipe deadlock.
    // If the child writes enough to fill the OS pipe buffer (~64KB) before
    // exiting, it will block on write() and wait_timeout will never return.
    let stdout_reader = child.stdout.take().map(OutputReader::start);
    let stderr_reader = child.stderr.take().map(OutputReader::start);

    let wait_result = child.wait_timeout(timeout);
    let (status, exit_status) = match wait_result {
        Ok(Some(status)) => {
            PROCESS_GROUPS.lock().unwrap().remove(&group);
            let execution_status = if status.success() {
                ExecutionStatus::Passed
            } else {
                ExecutionStatus::Failed
            };
            (execution_status, status.code().unwrap_or(-1))
        }
        Ok(None) => {
            // Timed out - kill the group before reaping, so its ID stays valid
            kill_process_group(group);
            let _ = child.kill();
            let _ = child.wait();
            PROCESS_GROUPS.lock().unwrap().remove(&group);
            (ExecutionStatus::Timedout, -1)
        }
        Err(e) => {
            PROCESS_GROUPS.lock().unwrap().remove(&group);
            kill_process_group(group);
            return Err(RunError::Wait(e));
        }
    };
    let deadline = Instant::now() + OUTPUT_DRAIN_TIMEOUT;
    let collect =
        |reader: Option<OutputReader>| reader.map(|r| r.collect(deadline)).unwrap_or_default();
    let result = ExecutionResult {
        status,
        exit_status,
        running_time: start.elapsed().as_millis() as i64,
        stdout_output: collect(stdout_reader),
        stderr_output: collect(stderr_reader),
    };
    // Set before the handler kills anything, so any death it caused is seen here
    if interrupted.load(Ordering::SeqCst) {
        return Err(RunError::Interrupted);
    }
    Ok(result)
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
        )
        .unwrap();
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
        )
        .unwrap();
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
        )
        .unwrap();
        assert_eq!(result.status, ExecutionStatus::Passed);
        assert_eq!(result.stdout_output.trim(), "done");
        assert!(result.running_time < 5000, "took {}ms", result.running_time);
    }

    #[test]
    fn spawn_failure_is_an_error() {
        let result = run_program(
            "/nonexistent/mull-test-program",
            &[],
            &[],
            Duration::from_secs(1),
            true,
        );
        assert!(matches!(result, Err(RunError::Spawn(_))), "{:?}", result);
    }

    #[test]
    fn nothing_runs_after_interrupt() {
        let interrupted = AtomicBool::new(true);
        let result = run("true", &[], &[], Duration::from_secs(1), true, &interrupted);
        assert!(matches!(result, Err(RunError::Interrupted)), "{:?}", result);
    }

    /// A program dying after the interrupt must not be reported as failed.
    #[test]
    fn death_after_interrupt_is_not_a_result() {
        let interrupted = AtomicBool::new(false);
        let result = std::thread::scope(|scope| {
            scope.spawn(|| {
                std::thread::sleep(Duration::from_millis(100));
                interrupted.store(true, Ordering::SeqCst);
            });
            run(
                "sh",
                &["-c".to_string(), "sleep 1; kill -9 $$".to_string()],
                &[],
                Duration::from_secs(10),
                true,
                &interrupted,
            )
        });
        assert!(matches!(result, Err(RunError::Interrupted)), "{:?}", result);
    }
}
