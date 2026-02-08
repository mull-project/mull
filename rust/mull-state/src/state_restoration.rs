use crate::{ExecutionResult, ExecutionState, ExecutionStatus, Mutant};
use mull_core::diag_error;
use mull_core::diagnostics::MullDiagnostics;
use rusqlite::{Connection, Result as SqliteResult};
use std::collections::HashMap;

#[derive(Debug)]
struct RawReport {
    pub info: HashMap<String, String>,
    pub execution_results: HashMap<Mutant, Vec<ExecutionResult>>,
}

fn load_raw_report(diag: &MullDiagnostics, database_path: &str) -> SqliteResult<RawReport> {
    let conn = Connection::open(database_path)?;

    let mut info = HashMap::new();
    let mut stmt = conn.prepare("SELECT key, value FROM information")?;
    let rows = stmt.query_map([], |row| {
        Ok((row.get::<_, String>(0)?, row.get::<_, String>(1)?))
    })?;
    for row in rows {
        let (key, value) = row?;
        info.insert(key, value);
    }

    let mut execution_results: HashMap<Mutant, Vec<ExecutionResult>> = HashMap::new();
    let mut stmt = conn.prepare(
        "SELECT mutant_id, execution_status, exit_status, duration, stdout, stderr, mutation_replacement FROM mutant",
    )?;
    let rows = stmt.query_map([], |row| {
        let mutant_id: String = row.get(0)?;
        let execution_status: i32 = row.get(1)?;
        let exit_status: i32 = row.get(2)?;
        let duration: i64 = row.get(3)?;
        let stdout: String = row.get(4)?;
        let stderr: String = row.get(5)?;
        let replacement: String = row.get(6)?;

        let execution_result = ExecutionResult {
            status: ExecutionStatus::from_i32(execution_status),
            exit_status: exit_status as i32,
            running_time: duration,
            stdout_output: stdout,
            stderr_output: stderr,
        };

        Ok((mutant_id, execution_result, replacement))
    })?;

    for row in rows {
        let (identifier, execution_result, replacement) = row?;
        execution_results
            .entry(Mutant::from_identifier(diag, identifier, replacement))
            .or_default()
            .push(execution_result);
    }

    Ok(RawReport {
        info,
        execution_results,
    })
}

fn status_priority(status: ExecutionStatus) -> u8 {
    match status {
        ExecutionStatus::Failed => 0,
        ExecutionStatus::Crashed => 1,
        ExecutionStatus::Timedout => 2,
        ExecutionStatus::AbnormalExit => 3,
        ExecutionStatus::FailFast => 4,
        ExecutionStatus::NotCovered => 5,
        ExecutionStatus::DryRun => 6,
        ExecutionStatus::Passed => 7,
        ExecutionStatus::Invalid => 8,
    }
}
// In cases when the same mutant is run against different tests we are only concerned with a single
// most appropriate execution result which is determined by the execution status as defined above
fn select_execution_result(results: Vec<ExecutionResult>) -> ExecutionResult {
    results
        .into_iter()
        .min_by_key(|r| status_priority(r.status))
        .expect("Cannot select execution result. Is the vector empty?")
}
pub fn restore_execution_state(diag: &MullDiagnostics, database_path: &str) -> ExecutionState {
    let mut execution_state = ExecutionState::default();
    match load_raw_report(diag, database_path) {
        Ok(report) => {
            execution_state.info = report.info;
            for (mutant, results) in report.execution_results {
                execution_state
                    .execution_results
                    .insert(mutant, select_execution_result(results));
            }
            execution_state
        }
        Err(err) => {
            diag_error!(
                diag,
                "Cannot restore execution state from {}: {}",
                database_path,
                err
            );
        }
    }
}
