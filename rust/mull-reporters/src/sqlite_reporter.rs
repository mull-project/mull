use crate::Reporter;
use mull_core::diagnostics::MullDiagnostics;
use mull_core::{diag_error, diag_info};
use mull_state::ExecutionState;
use rusqlite::{named_params, Connection, Result as SqliteResult};
use std::fs;
use std::time::{SystemTime, UNIX_EPOCH};

const CREATE_TABLES: &str = r#"
CREATE TABLE IF NOT EXISTS mutant (
  mutant_id TEXT,
  mutator TEXT,
  filename TEXT,
  directory TEXT,
  line_number INT,
  column_number INT,
  end_line_number INT,
  end_column_number INT,
  execution_status INT,
  exit_status INT,
  duration INT,
  stdout TEXT,
  stderr TEXT,
  mutation_replacement TEXT
);

CREATE TABLE IF NOT EXISTS information (
  key TEXT,
  value TEXT
);
"#;

const INSERT_INFO: &str = "INSERT INTO information (key, value) VALUES (:key, :value)";

const INSERT_MUTANT: &str = r#"
INSERT INTO mutant (
  mutant_id, mutator, filename, directory,
  line_number, column_number, end_line_number, end_column_number,
  execution_status, exit_status, duration,
  stdout, stderr, mutation_replacement
) VALUES (
  :mutant_id, :mutator, :filename, :directory,
  :line_number, :column_number, :end_line_number, :end_column_number,
  :execution_status, :exit_status, :duration,
  :stdout, :stderr, :replacement
)
"#;

pub struct SQLiteReporter {
    database_path: String,
}

impl SQLiteReporter {
    pub fn new(report_dir: &str, report_name: &str) -> Self {
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
        }
    }

    fn write_report(&self, state: &ExecutionState) -> SqliteResult<()> {
        let mut conn = Connection::open(&self.database_path)?;

        conn.execute_batch(
            "
            PRAGMA journal_mode = OFF;
            PRAGMA synchronous = OFF;
            PRAGMA cache_size = 10000;
            PRAGMA temp_store = MEMORY;
        ",
        )?;

        conn.execute_batch(CREATE_TABLES)?;

        let tx = conn.transaction()?;

        {
            let mut stmt = tx.prepare_cached(INSERT_INFO)?;
            for (key, value) in &state.info {
                stmt.execute(named_params! { ":key": key, ":value": value })?;
            }
        }

        {
            let mut stmt = tx.prepare_cached(INSERT_MUTANT)?;
            for (mutant, result) in &state.execution_results {
                let loc = &mutant.location;
                stmt.execute(named_params! {
                    ":mutant_id": &mutant.identifier,
                    ":mutator": &mutant.mutator,
                    ":filename": &loc.filepath,
                    ":directory": "",
                    ":line_number": loc.begin_line,
                    ":column_number": loc.begin_column,
                    ":end_line_number": loc.end_line,
                    ":end_column_number": loc.end_column,
                    ":execution_status": result.status as i32,
                    ":exit_status": result.exit_status,
                    ":duration": result.running_time,
                    ":stdout": &result.stdout_output,
                    ":stderr": &result.stderr_output,
                    ":replacement": &mutant.replacement,
                })?;
            }
        }

        tx.commit()
    }
}

impl Reporter for SQLiteReporter {
    fn report(&self, diag: &MullDiagnostics, state: &ExecutionState) {
        assert!(!state.execution_results.is_empty());
        match self.write_report(state) {
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
