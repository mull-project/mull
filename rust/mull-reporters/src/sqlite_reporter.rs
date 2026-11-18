use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_error, diag_info};
use mull_state::sqlite::write_report;
use mull_state::ExecutionState;
use std::fs;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

pub struct SQLiteReporter {
    database_path: String,
    busy_timeout: Duration,
}

impl SQLiteReporter {
    pub fn new(report_dir: &str, report_name: &str, busy_timeout_ms: u32) -> Self {
        let dir = if report_dir.is_empty() {
            "."
        } else {
            report_dir
        };
        let _ = fs::create_dir_all(dir);

        let name = if report_name.is_empty() {
            let ts = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|d| d.as_secs())
                .unwrap_or(0);
            ts.to_string()
        } else {
            report_name.to_string()
        };

        Self {
            database_path: format!("{}/{}.sqlite", dir, name),
            busy_timeout: Duration::from_millis(busy_timeout_ms as u64),
        }
    }
}

impl Reporter for SQLiteReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());
        match write_report(&self.database_path, self.busy_timeout, state) {
            Ok(()) => diag_info!(diag, "Results can be found at '{}'", self.database_path),
            Err(err) => diag_error!(
                diag,
                "Failed to write SQLite report to '{}': {}",
                self.database_path,
                err
            ),
        }
    }
}
