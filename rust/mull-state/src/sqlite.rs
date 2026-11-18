use crate::{ExecutionResult, ExecutionState, ExecutionStatus, Mutant};
use mull_core::diag_error;
use mull_core::diagnostics::MullDiagnostics;
use rusqlite::{params, Connection, OptionalExtension, Result as SqliteResult, Row};
use std::collections::HashMap;
use std::path::Path;
use std::time::Duration;

// run_identity is set by the runner cache only
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
  mutation_replacement TEXT,
  run_identity TEXT
);

CREATE TABLE IF NOT EXISTS information (
  key TEXT,
  value TEXT
);
"#;

const INSERT_MUTANT: &str = r#"
INSERT INTO mutant (
  mutant_id, mutator, filename, directory,
  line_number, column_number, end_line_number, end_column_number,
  execution_status, exit_status, duration,
  stdout, stderr, mutation_replacement, run_identity
) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15)
"#;

/// Columns of a stored result, in the order `read_result` expects
const RESULT_COLUMNS: &str =
    "mutant_id, mutation_replacement, execution_status, exit_status, duration, stdout, stderr";

/// (mutant id, replacement, result) from a row selected with `RESULT_COLUMNS`
fn read_result(row: &Row) -> SqliteResult<(String, String, ExecutionResult)> {
    Ok((
        row.get(0)?,
        row.get(1)?,
        ExecutionResult {
            status: ExecutionStatus::from_i32(row.get(2)?),
            exit_status: row.get(3)?,
            running_time: row.get(4)?,
            stdout_output: row.get(5)?,
            stderr_output: row.get(6)?,
        },
    ))
}

/// `run_identity` is set for cached results, NULL in reports
fn insert_result(
    conn: &Connection,
    mutant: &Mutant,
    result: &ExecutionResult,
    run_identity: Option<&str>,
) -> SqliteResult<()> {
    let loc = &mutant.location;
    conn.prepare_cached(INSERT_MUTANT)?.execute(params![
        mutant.identifier,
        mutant.mutator,
        loc.filepath,
        "",
        loc.begin_line as i64,
        loc.begin_column as i64,
        loc.end_line as i64,
        loc.end_column as i64,
        result.status as i32,
        result.exit_status,
        result.running_time,
        result.stdout_output,
        result.stderr_output,
        mutant.replacement,
        run_identity,
    ])?;
    Ok(())
}

pub fn write_report(
    path: &str,
    busy_timeout: Duration,
    state: &ExecutionState,
) -> SqliteResult<()> {
    let mut conn = Connection::open(path)?;
    conn.busy_timeout(busy_timeout)?;
    conn.execute_batch(
        "PRAGMA journal_mode = OFF; PRAGMA synchronous = OFF; PRAGMA cache_size = 10000; PRAGMA temp_store = MEMORY;",
    )?;
    conn.execute_batch(CREATE_TABLES)?;
    let tx = conn.transaction()?;
    {
        let mut stmt = tx.prepare_cached("INSERT INTO information (key, value) VALUES (?1, ?2)")?;
        for (key, value) in &state.info {
            stmt.execute([key, value])?;
        }
    }
    for (mutant, result) in &state.execution_results {
        insert_result(&tx, mutant, result, None)?;
    }
    tx.commit()
}

/// Statuses that prove a mutant is killed regardless of which program ran it
fn killed_statuses() -> String {
    [
        ExecutionStatus::Failed,
        ExecutionStatus::Crashed,
        ExecutionStatus::AbnormalExit,
        ExecutionStatus::FailFast,
    ]
    .iter()
    .map(|s| (*s as i32).to_string())
    .collect::<Vec<_>>()
    .join(",")
}

/// A connection to a cache shared by concurrent runners
pub fn connect_cache(path: &Path, busy_timeout: Duration) -> SqliteResult<Connection> {
    let conn = Connection::open(path)?;
    conn.busy_timeout(busy_timeout)?;
    conn.execute_batch("PRAGMA journal_mode = WAL; PRAGMA synchronous = NORMAL;")?;
    Ok(conn)
}

pub fn prepare_cache(conn: &Connection) -> SqliteResult<()> {
    conn.execute_batch(CREATE_TABLES)?;
    conn.execute_batch(
        "CREATE INDEX IF NOT EXISTS mutant_by_id ON mutant(mutant_id, mutation_replacement);",
    )
}

/// Own results and foreign kills, keyed by (mutant id, replacement)
pub fn load_cached(
    conn: &Connection,
    identity: &str,
) -> SqliteResult<HashMap<(String, String), ExecutionResult>> {
    let mut cached = HashMap::new();
    // Foreign kills first: a kill elsewhere outranks a survivor here
    let mut stmt = conn.prepare(&format!(
        "SELECT {} FROM mutant WHERE run_identity != ?1 AND execution_status IN ({}) ORDER BY rowid DESC",
        RESULT_COLUMNS,
        killed_statuses()
    ))?;
    for row in stmt.query_map([identity], read_result)? {
        let (id, replacement, result) = row?;
        // The oldest kill wins, so the choice does not depend on row order
        cached.insert((id, replacement), result);
    }
    let mut stmt = conn.prepare(&format!(
        "SELECT {} FROM mutant WHERE run_identity = ?1",
        RESULT_COLUMNS
    ))?;
    for row in stmt.query_map([identity], read_result)? {
        let (id, replacement, result) = row?;
        let key = (id, replacement);
        if result.status != ExecutionStatus::Passed || !cached.contains_key(&key) {
            cached.insert(key, result);
        }
    }
    Ok(cached)
}

/// A kill recorded by another run, e.g. by a concurrent runner
pub fn foreign_kill(
    conn: &Connection,
    identity: &str,
    mutant: &Mutant,
) -> SqliteResult<Option<ExecutionResult>> {
    conn.prepare_cached(&format!(
        "SELECT {} FROM mutant WHERE mutant_id = ?1 AND mutation_replacement = ?2 AND run_identity != ?3 AND execution_status IN ({}) ORDER BY rowid LIMIT 1",
        RESULT_COLUMNS,
        killed_statuses()
    ))?
    .query_row([&mutant.identifier, &mutant.replacement, identity], read_result)
    .optional()
    .map(|row| row.map(|(_, _, result)| result))
}

pub fn store_cached(
    conn: &Connection,
    identity: &str,
    mutant: &Mutant,
    result: &ExecutionResult,
) -> SqliteResult<()> {
    insert_result(conn, mutant, result, Some(identity))
}

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
    let mut stmt = conn.prepare(&format!("SELECT {} FROM mutant", RESULT_COLUMNS))?;
    for row in stmt.query_map([], read_result)? {
        let (identifier, replacement, execution_result) = row?;
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
