use mull_state::{ExecutionResult, ExecutionStatus, Mutant};
use rusqlite::{params, Connection, OptionalExtension, Row, TransactionBehavior};
use std::collections::HashMap;
use std::fs::File;
use std::io::{self, Read};
use std::path::{Path, PathBuf};
use std::sync::Mutex;
use std::time::Duration;
use twox_hash::XxHash3_128;

const SCHEMA_VERSION: i64 = 1;

const SCHEMA: &str = r#"
CREATE TABLE run (
  id INTEGER PRIMARY KEY,
  identity TEXT UNIQUE NOT NULL,
  fingerprint TEXT NOT NULL
);
CREATE TABLE result (
  run_id INTEGER NOT NULL REFERENCES run(id) ON DELETE CASCADE,
  mutant_id TEXT NOT NULL,
  replacement TEXT NOT NULL,
  status INTEGER NOT NULL,
  exit_status INTEGER NOT NULL,
  duration INTEGER NOT NULL,
  stdout TEXT NOT NULL,
  stderr TEXT NOT NULL,
  PRIMARY KEY (run_id, mutant_id, replacement)
);
CREATE INDEX result_by_mutant ON result(mutant_id, replacement);
"#;

const DROP_SCHEMA: &str = r#"
DROP TABLE IF EXISTS result;
DROP TABLE IF EXISTS run;
"#;

const RESULT_COLUMNS: &str =
    "mutant_id, replacement, status, exit_status, duration, stdout, stderr";

/// Statuses that prove a mutant is killed regardless of which program ran it
const KILLED: [ExecutionStatus; 4] = [
    ExecutionStatus::Failed,
    ExecutionStatus::Crashed,
    ExecutionStatus::AbnormalExit,
    ExecutionStatus::FailFast,
];

fn killed_list() -> String {
    KILLED
        .iter()
        .map(|s| (*s as i32).to_string())
        .collect::<Vec<_>>()
        .join(",")
}

/// What a run is: the cache file it uses by default and its row in a shared cache
#[derive(Debug, Clone)]
pub struct RunIdentity {
    pub executable: String,
    pub test_program: String,
    pub args: Vec<String>,
    pub dylibs: Vec<String>,
    pub working_directory: String,
}

impl RunIdentity {
    pub fn key(&self) -> String {
        fn list(hasher: &mut XxHash3_128, items: &[String]) {
            hasher.write(&(items.len() as u64).to_le_bytes());
            for item in items {
                hasher.write(&(item.len() as u64).to_le_bytes());
                hasher.write(item.as_bytes());
            }
        }
        let mut dylibs = self.dylibs.clone();
        dylibs.sort();
        let mut hasher = XxHash3_128::new();
        list(
            &mut hasher,
            &[
                self.executable.clone(),
                self.test_program.clone(),
                self.working_directory.clone(),
            ],
        );
        list(&mut hasher, &self.args);
        list(&mut hasher, &dylibs);
        format!("{:032x}", hasher.finish_128())
    }
}

/// Content hash of the files a run depends on
pub fn fingerprint<P: AsRef<Path>>(files: &[P]) -> io::Result<String> {
    let mut hasher = XxHash3_128::new();
    let mut buffer = vec![0; 1 << 20];
    for path in files {
        let path = path.as_ref();
        let path_bytes = path.as_os_str().as_encoded_bytes();
        hasher.write(&(path_bytes.len() as u64).to_le_bytes());
        hasher.write(path_bytes);
        let mut file = File::open(path)?;
        let mut size = 0u64;
        loop {
            let count = file.read(&mut buffer)?;
            if count == 0 {
                break;
            }
            hasher.write(&buffer[..count]);
            size += count as u64;
        }
        hasher.write(&size.to_le_bytes());
    }
    Ok(format!("{:032x}", hasher.finish_128()))
}

/// `$XDG_CACHE_HOME/mull/<identity>.sqlite`, falling back to `~/.cache`
pub fn default_cache_path(identity: &RunIdentity) -> Option<PathBuf> {
    let base = std::env::var_os("XDG_CACHE_HOME")
        .filter(|v| !v.is_empty())
        .map(PathBuf::from)
        .or_else(|| {
            std::env::var_os("HOME")
                .filter(|v| !v.is_empty())
                .map(|home| PathBuf::from(home).join(".cache"))
        })?;
    Some(base.join("mull").join(format!("{}.sqlite", identity.key())))
}

pub struct Cache {
    path: PathBuf,
    busy_timeout: Duration,
    run_id: i64,
    connections: Mutex<Vec<Connection>>,
}

fn connect(path: &Path, busy_timeout: Duration) -> rusqlite::Result<Connection> {
    let conn = Connection::open(path)?;
    conn.busy_timeout(busy_timeout)?;
    conn.execute_batch(
        "PRAGMA journal_mode = WAL; PRAGMA synchronous = NORMAL; PRAGMA foreign_keys = ON;",
    )?;
    Ok(conn)
}

fn read_result(row: &Row) -> rusqlite::Result<(String, String, ExecutionResult)> {
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

/// Results a run may reuse: its own, or a kill from any other run
pub enum Cached {
    Own(ExecutionResult),
    Foreign(ExecutionResult),
}

impl Cached {
    pub fn into_result(self) -> ExecutionResult {
        match self {
            Cached::Own(r) | Cached::Foreign(r) => r,
        }
    }
}

impl Cache {
    /// Registers the run; its old results are dropped when the fingerprint changed
    pub fn open(
        path: &Path,
        identity: &str,
        fingerprint: &str,
        busy_timeout: Duration,
    ) -> rusqlite::Result<Self> {
        let mut conn = connect(path, busy_timeout)?;
        let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
        let version: i64 = tx.query_row("PRAGMA user_version", [], |r| r.get(0))?;
        if version != SCHEMA_VERSION {
            tx.execute_batch(DROP_SCHEMA)?;
            tx.execute_batch(SCHEMA)?;
            tx.execute_batch(&format!("PRAGMA user_version = {}", SCHEMA_VERSION))?;
        }
        let existing: Option<(i64, String)> = tx
            .query_row(
                "SELECT id, fingerprint FROM run WHERE identity = ?1",
                [identity],
                |r| Ok((r.get(0)?, r.get(1)?)),
            )
            .optional()?;
        let run_id = match existing {
            Some((id, stored)) if stored == fingerprint => id,
            Some((id, _)) => {
                tx.execute("DELETE FROM result WHERE run_id = ?1", [id])?;
                tx.execute(
                    "UPDATE run SET fingerprint = ?1 WHERE id = ?2",
                    params![fingerprint, id],
                )?;
                id
            }
            None => {
                tx.execute(
                    "INSERT INTO run (identity, fingerprint) VALUES (?1, ?2)",
                    [identity, fingerprint],
                )?;
                tx.last_insert_rowid()
            }
        };
        tx.commit()?;
        Ok(Self {
            path: path.to_path_buf(),
            busy_timeout,
            run_id,
            connections: Mutex::new(vec![conn]),
        })
    }

    fn with_connection<T>(
        &self,
        f: impl FnOnce(&Connection) -> rusqlite::Result<T>,
    ) -> rusqlite::Result<T> {
        let pooled = self.connections.lock().unwrap().pop();
        let conn = match pooled {
            Some(c) => c,
            None => connect(&self.path, self.busy_timeout)?,
        };
        let result = f(&conn);
        self.connections.lock().unwrap().push(conn);
        result
    }

    /// Everything reusable for this run, keyed by (mutant id, replacement)
    pub fn load(&self) -> rusqlite::Result<HashMap<(String, String), Cached>> {
        self.with_connection(|conn| {
            let mut cached = HashMap::new();
            // Foreign kills first: a kill elsewhere outranks a survivor here
            let mut stmt = conn.prepare(&format!(
                "SELECT {} FROM result WHERE run_id != ?1 AND status IN ({}) ORDER BY run_id DESC",
                RESULT_COLUMNS,
                killed_list()
            ))?;
            for row in stmt.query_map([self.run_id], read_result)? {
                let (id, replacement, result) = row?;
                // Lowest run id wins, so the choice does not depend on row order
                cached.insert((id, replacement), Cached::Foreign(result));
            }
            let mut stmt = conn.prepare(&format!(
                "SELECT {} FROM result WHERE run_id = ?1",
                RESULT_COLUMNS
            ))?;
            for row in stmt.query_map([self.run_id], read_result)? {
                let (id, replacement, result) = row?;
                let key = (id, replacement);
                if result.status != ExecutionStatus::Passed || !cached.contains_key(&key) {
                    cached.insert(key, Cached::Own(result));
                }
            }
            Ok(cached)
        })
    }

    /// A kill recorded by another run since `load`, e.g. by a concurrent runner
    pub fn foreign_kill(&self, mutant: &Mutant) -> rusqlite::Result<Option<ExecutionResult>> {
        self.with_connection(|conn| {
            let mut stmt = conn.prepare_cached(&format!(
                "SELECT {} FROM result WHERE mutant_id = ?1 AND replacement = ?2 AND run_id != ?3 AND status IN ({}) ORDER BY run_id LIMIT 1",
                RESULT_COLUMNS,
                killed_list()
            ))?;
            stmt.query_row(
                params![mutant.identifier, mutant.replacement, self.run_id],
                read_result,
            )
            .optional()
            .map(|row| row.map(|(_, _, result)| result))
        })
    }

    pub fn store(&self, mutant: &Mutant, result: &ExecutionResult) -> rusqlite::Result<()> {
        if !is_cacheable(result.status) {
            return Ok(());
        }
        self.with_connection(|conn| {
            let mut stmt = conn.prepare_cached(&format!(
                "INSERT OR REPLACE INTO result (run_id, {}) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)",
                RESULT_COLUMNS
            ))?;
            stmt.execute(params![
                self.run_id,
                mutant.identifier,
                mutant.replacement,
                result.status as i32,
                result.exit_status,
                result.running_time,
                result.stdout_output,
                result.stderr_output,
            ])?;
            Ok(())
        })
    }
}

/// Only outcomes of an actual mutant run
fn is_cacheable(status: ExecutionStatus) -> bool {
    status == ExecutionStatus::Passed
        || status == ExecutionStatus::Timedout
        || KILLED.contains(&status)
}

#[cfg(test)]
mod tests {
    use super::*;
    use mull_core::MullDiagnostics;

    struct TempDir(PathBuf);

    impl TempDir {
        fn new(name: &str) -> Self {
            let path = std::env::temp_dir().join(format!(
                "mull-cache-test-{}-{}",
                name,
                std::process::id()
            ));
            let _ = std::fs::remove_dir_all(&path);
            std::fs::create_dir_all(&path).unwrap();
            Self(path)
        }

        fn db(&self) -> PathBuf {
            self.0.join("cache.sqlite")
        }
    }

    impl Drop for TempDir {
        fn drop(&mut self) {
            let _ = std::fs::remove_dir_all(&self.0);
        }
    }

    fn mutant(line: u32) -> Mutant {
        let diag = MullDiagnostics::with_writer(Box::new(std::io::sink()), false);
        Mutant::from_identifier(
            &diag,
            format!("cxx_add_to_sub:/src/a.c:{}:1:{}:5", line, line),
            "-".into(),
        )
    }

    fn result(status: ExecutionStatus) -> ExecutionResult {
        ExecutionResult {
            status,
            exit_status: 1,
            running_time: 7,
            stdout_output: "out".into(),
            stderr_output: "err".into(),
        }
    }

    fn open(dir: &TempDir, identity: &str, fingerprint: &str) -> Cache {
        Cache::open(&dir.db(), identity, fingerprint, Duration::from_secs(5)).unwrap()
    }

    fn key(m: &Mutant) -> (String, String) {
        (m.identifier.clone(), m.replacement.clone())
    }

    #[test]
    fn own_results_survive_a_restart() {
        let dir = TempDir::new("restart");
        let cache = open(&dir, "a", "f1");
        cache
            .store(&mutant(1), &result(ExecutionStatus::Failed))
            .unwrap();
        cache
            .store(&mutant(2), &result(ExecutionStatus::Passed))
            .unwrap();
        cache
            .store(&mutant(3), &result(ExecutionStatus::Timedout))
            .unwrap();
        drop(cache);

        let loaded = open(&dir, "a", "f1").load().unwrap();
        assert_eq!(loaded.len(), 3);
        let Some(Cached::Own(r)) = loaded.get(&key(&mutant(2))) else {
            panic!("own survivor not reused")
        };
        assert_eq!(r.status, ExecutionStatus::Passed);
        assert_eq!(r.stdout_output, "out");
        assert_eq!(r.stderr_output, "err");
        assert_eq!(r.running_time, 7);
    }

    #[test]
    fn new_fingerprint_drops_own_results() {
        let dir = TempDir::new("fingerprint");
        open(&dir, "a", "f1")
            .store(&mutant(1), &result(ExecutionStatus::Failed))
            .unwrap();
        assert!(open(&dir, "a", "f2").load().unwrap().is_empty());
        assert!(open(&dir, "a", "f1").load().unwrap().is_empty());
    }

    #[test]
    fn only_kills_are_shared_between_runs() {
        let dir = TempDir::new("shared");
        let a = open(&dir, "a", "f");
        a.store(&mutant(1), &result(ExecutionStatus::Failed))
            .unwrap();
        a.store(&mutant(2), &result(ExecutionStatus::Passed))
            .unwrap();
        a.store(&mutant(3), &result(ExecutionStatus::Timedout))
            .unwrap();

        let b = open(&dir, "b", "f");
        let loaded = b.load().unwrap();
        assert_eq!(loaded.len(), 1);
        assert!(matches!(
            loaded.get(&key(&mutant(1))),
            Some(Cached::Foreign(_))
        ));
        assert!(b.foreign_kill(&mutant(1)).unwrap().is_some());
        assert!(b.foreign_kill(&mutant(2)).unwrap().is_none());
        assert!(b.foreign_kill(&mutant(3)).unwrap().is_none());
    }

    #[test]
    fn foreign_kill_outranks_own_survivor() {
        let dir = TempDir::new("outrank");
        let b = open(&dir, "b", "f");
        b.store(&mutant(1), &result(ExecutionStatus::Passed))
            .unwrap();
        b.store(&mutant(2), &result(ExecutionStatus::Timedout))
            .unwrap();
        let a = open(&dir, "a", "f");
        a.store(&mutant(1), &result(ExecutionStatus::Failed))
            .unwrap();
        a.store(&mutant(2), &result(ExecutionStatus::Failed))
            .unwrap();

        let loaded = b.load().unwrap();
        assert!(matches!(
            loaded.get(&key(&mutant(1))),
            Some(Cached::Foreign(_))
        ));
        assert!(matches!(loaded.get(&key(&mutant(2))), Some(Cached::Own(_))));
    }

    #[test]
    fn non_execution_statuses_are_not_stored() {
        let dir = TempDir::new("statuses");
        let cache = open(&dir, "a", "f");
        for status in [
            ExecutionStatus::NotCovered,
            ExecutionStatus::DryRun,
            ExecutionStatus::Invalid,
        ] {
            cache.store(&mutant(1), &result(status)).unwrap();
        }
        assert!(cache.load().unwrap().is_empty());
    }

    #[test]
    fn identity_key_separates_fields() {
        let base = RunIdentity {
            executable: "/a".into(),
            test_program: "/a".into(),
            args: vec!["x".into()],
            dylibs: vec!["/l1".into(), "/l2".into()],
            working_directory: "/w".into(),
        };
        let mut reordered = base.clone();
        reordered.dylibs.reverse();
        assert_eq!(base.key(), reordered.key());

        let mut arg_as_dylib = base.clone();
        arg_as_dylib.args.clear();
        arg_as_dylib.dylibs.insert(0, "x".into());
        assert_ne!(base.key(), arg_as_dylib.key());

        let mut other_args = base.clone();
        other_args.args.push("y".into());
        assert_ne!(base.key(), other_args.key());
    }

    #[test]
    fn fingerprint_follows_content() {
        let dir = TempDir::new("content");
        let file = dir.0.join("exe");
        std::fs::write(&file, b"one").unwrap();
        let first = fingerprint(&[&file]).unwrap();
        assert_eq!(first, fingerprint(&[&file]).unwrap());
        std::fs::write(&file, b"two").unwrap();
        assert_ne!(first, fingerprint(&[&file]).unwrap());
    }
}
