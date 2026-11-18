use mull_state::{sqlite, ExecutionResult, ExecutionStatus, Mutant};
use rusqlite::Connection;
use std::collections::HashMap;
use std::fs::File;
use std::io::{self, Read};
use std::path::{Path, PathBuf};
use std::sync::Mutex;
use std::time::Duration;
use twox_hash::XxHash3_128;

/// What a run is: the cache file it uses by default and its rows in a shared cache
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

/// `<temp dir>/mull/<identity>-<fingerprint>.sqlite`, a rebuild gets a new file
pub fn default_cache_path(identity: &RunIdentity, fingerprint: &str) -> PathBuf {
    std::env::temp_dir()
        .join("mull")
        .join(format!("{}-{}.sqlite", identity.key(), fingerprint))
}

pub struct Cache {
    path: PathBuf,
    busy_timeout: Duration,
    identity: String,
    dropped: usize,
    connections: Mutex<Vec<Connection>>,
}

impl Cache {
    /// Kills outlive a rebuild, other results of the run are tried again
    pub fn open(
        path: &Path,
        identity: &str,
        fingerprint: &str,
        busy_timeout: Duration,
    ) -> rusqlite::Result<Self> {
        let mut conn = sqlite::connect_cache(path, busy_timeout)?;
        sqlite::prepare_cache(&conn)?;
        let dropped = sqlite::refresh_run(&mut conn, identity, fingerprint)?;
        Ok(Self {
            path: path.to_path_buf(),
            busy_timeout,
            identity: identity.to_string(),
            dropped,
            connections: Mutex::new(vec![conn]),
        })
    }

    /// Results dropped on open because the run's files changed
    pub fn dropped(&self) -> usize {
        self.dropped
    }

    fn with_connection<T>(
        &self,
        f: impl FnOnce(&Connection) -> rusqlite::Result<T>,
    ) -> rusqlite::Result<T> {
        let pooled = self.connections.lock().unwrap().pop();
        let conn = match pooled {
            Some(c) => c,
            None => sqlite::connect_cache(&self.path, self.busy_timeout)?,
        };
        let result = f(&conn);
        self.connections.lock().unwrap().push(conn);
        result
    }

    /// Own results and foreign kills, keyed by (mutant id, replacement)
    pub fn load(&self) -> rusqlite::Result<HashMap<(String, String), ExecutionResult>> {
        self.with_connection(|conn| sqlite::load_cached(conn, &self.identity))
    }

    /// A kill recorded by another run since `load`, e.g. by a concurrent runner
    pub fn foreign_kill(&self, mutant: &Mutant) -> rusqlite::Result<Option<ExecutionResult>> {
        self.with_connection(|conn| sqlite::foreign_kill(conn, &self.identity, mutant))
    }

    pub fn store(&self, mutant: &Mutant, result: &ExecutionResult) -> rusqlite::Result<()> {
        if !is_cacheable(result.status) {
            return Ok(());
        }
        self.with_connection(|conn| sqlite::store_cached(conn, &self.identity, mutant, result))
    }
}

/// Only outcomes of an actual mutant run
fn is_cacheable(status: ExecutionStatus) -> bool {
    !matches!(
        status,
        ExecutionStatus::NotCovered | ExecutionStatus::DryRun | ExecutionStatus::Invalid
    )
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

    fn open(dir: &TempDir, identity: &str) -> Cache {
        open_built(dir, identity, "build")
    }

    fn open_built(dir: &TempDir, identity: &str, fingerprint: &str) -> Cache {
        Cache::open(&dir.db(), identity, fingerprint, Duration::from_secs(5)).unwrap()
    }

    fn key(m: &Mutant) -> (String, String) {
        (m.identifier.clone(), m.replacement.clone())
    }

    #[test]
    fn own_results_survive_a_restart() {
        let dir = TempDir::new("restart");
        let cache = open(&dir, "a");
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

        let loaded = open(&dir, "a").load().unwrap();
        assert_eq!(loaded.len(), 3);
        let r = &loaded[&key(&mutant(2))];
        assert_eq!(r.status, ExecutionStatus::Passed);
        assert_eq!(r.stdout_output, "out");
        assert_eq!(r.stderr_output, "err");
        assert_eq!(r.running_time, 7);
    }

    #[test]
    fn only_kills_are_shared_between_runs() {
        let dir = TempDir::new("shared");
        let a = open(&dir, "a");
        a.store(&mutant(1), &result(ExecutionStatus::Failed))
            .unwrap();
        a.store(&mutant(2), &result(ExecutionStatus::Passed))
            .unwrap();
        a.store(&mutant(3), &result(ExecutionStatus::Timedout))
            .unwrap();

        let b = open(&dir, "b");
        let loaded = b.load().unwrap();
        assert_eq!(loaded.len(), 1);
        assert_eq!(loaded[&key(&mutant(1))].status, ExecutionStatus::Failed);
        assert!(b.foreign_kill(&mutant(1)).unwrap().is_some());
        assert!(b.foreign_kill(&mutant(2)).unwrap().is_none());
        assert!(b.foreign_kill(&mutant(3)).unwrap().is_none());
    }

    #[test]
    fn foreign_kill_outranks_own_survivor() {
        let dir = TempDir::new("outrank");
        let b = open(&dir, "b");
        b.store(&mutant(1), &result(ExecutionStatus::Passed))
            .unwrap();
        b.store(&mutant(2), &result(ExecutionStatus::Timedout))
            .unwrap();
        let a = open(&dir, "a");
        a.store(&mutant(1), &result(ExecutionStatus::Failed))
            .unwrap();
        a.store(&mutant(2), &result(ExecutionStatus::Failed))
            .unwrap();

        let loaded = b.load().unwrap();
        assert_eq!(loaded[&key(&mutant(1))].status, ExecutionStatus::Failed);
        assert_eq!(loaded[&key(&mutant(2))].status, ExecutionStatus::Timedout);
    }

    #[test]
    fn non_execution_statuses_are_not_stored() {
        let dir = TempDir::new("statuses");
        let cache = open(&dir, "a");
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
    fn rebuild_keeps_only_kills() {
        let dir = TempDir::new("rebuild");
        let cache = open_built(&dir, "a", "one");
        assert_eq!(cache.dropped(), 0);
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

        let same = open_built(&dir, "a", "one");
        assert_eq!(same.dropped(), 0);
        assert_eq!(same.load().unwrap().len(), 3);
        drop(same);

        let rebuilt = open_built(&dir, "a", "two");
        assert_eq!(rebuilt.dropped(), 2);
        let loaded = rebuilt.load().unwrap();
        assert_eq!(loaded.len(), 1);
        assert_eq!(loaded[&key(&mutant(1))].status, ExecutionStatus::Failed);
    }

    #[test]
    fn rebuild_leaves_other_runs_alone() {
        let dir = TempDir::new("rebuild-shared");
        let b = open_built(&dir, "b", "one");
        b.store(&mutant(1), &result(ExecutionStatus::Passed))
            .unwrap();
        drop(b);

        assert_eq!(open_built(&dir, "a", "two").dropped(), 0);
        let loaded = open_built(&dir, "b", "one").load().unwrap();
        assert_eq!(loaded[&key(&mutant(1))].status, ExecutionStatus::Passed);
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
