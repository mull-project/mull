use std::fs;
use std::path::{Path, PathBuf};

const PLUGIN_PREFIX: &str = "mull-ir-frontend-";
/// Embedded by the plugin, see tools/mull-ir-frontend/mull-cxx-ir-frontend.cpp
const MULL_VERSION_MARKER: &str = "mull-plugin-version:";
const LLVM_VERSION_MARKER: &str = "mull-plugin-llvm:";

pub struct Plugin {
    pub path: PathBuf,
    pub mull_version: Option<String>,
    pub llvm_version: Option<String>,
}

/// Packages install plugins into `<prefix>/lib` next to `<prefix>/bin/mull-runner`.
/// Check the executable path both as invoked (Homebrew links) and with symlinks resolved.
fn plugin_dirs(exe: &Path) -> Vec<PathBuf> {
    let mut candidates = vec![exe.to_path_buf()];
    if let Ok(resolved) = exe.canonicalize() {
        candidates.push(resolved);
    }
    let mut dirs: Vec<PathBuf> = Vec::new();
    for candidate in candidates {
        let lib = candidate
            .parent()
            .and_then(Path::parent)
            .map(|p| p.join("lib"));
        if let Some(Ok(lib)) = lib.map(|l| l.canonicalize()) {
            if !dirs.contains(&lib) {
                dirs.push(lib);
            }
        }
    }
    dirs
}

fn llvm_major(file_name: &str) -> Option<u32> {
    file_name.strip_prefix(PLUGIN_PREFIX)?.parse().ok()
}

fn read_marker(data: &[u8], marker: &str) -> Option<String> {
    let marker = marker.as_bytes();
    let start = data.windows(marker.len()).position(|w| w == marker)? + marker.len();
    let len = data[start..].iter().position(|&b| b == 0)?;
    let value = std::str::from_utf8(&data[start..start + len]).ok()?;
    // Unstamped builds keep the workspace status placeholder
    if value.contains("{STABLE_") {
        return Some("dev".into());
    }
    Some(value.to_string())
}

fn find_plugins(dir: &Path) -> Vec<Plugin> {
    let Ok(entries) = fs::read_dir(dir) else {
        return Vec::new();
    };
    let mut found: Vec<(u32, PathBuf)> = entries
        .filter_map(|e| e.ok())
        .filter_map(|e| {
            let major = llvm_major(e.file_name().to_str()?)?;
            Some((major, e.path()))
        })
        .collect();
    found.sort();
    found
        .into_iter()
        .map(|(_, path)| {
            let data = fs::read(&path).unwrap_or_default();
            Plugin {
                mull_version: read_marker(&data, MULL_VERSION_MARKER),
                llvm_version: read_marker(&data, LLVM_VERSION_MARKER),
                path,
            }
        })
        .collect()
}

pub fn installed_plugins(exe: &Path) -> Vec<Plugin> {
    plugin_dirs(exe)
        .iter()
        .flat_map(|d| find_plugins(d))
        .collect()
}

pub fn describe_plugins(plugins: &[Plugin]) -> String {
    let mut out = String::from("Plugins:");
    if plugins.is_empty() {
        out.push_str("\n  none found");
    }
    for plugin in plugins {
        out.push_str(&format!(
            "\n  {} (mull {}, LLVM {})",
            plugin.path.display(),
            plugin.mull_version.as_deref().unwrap_or("unknown"),
            plugin.llvm_version.as_deref().unwrap_or("unknown"),
        ));
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn temp_prefix(name: &str) -> PathBuf {
        let dir =
            std::env::temp_dir().join(format!("mull-plugins-{}-{}", std::process::id(), name));
        let _ = fs::remove_dir_all(&dir);
        fs::create_dir_all(dir.join("bin")).unwrap();
        fs::create_dir_all(dir.join("lib")).unwrap();
        dir
    }

    #[test]
    fn reads_markers() {
        let data = b"junk\0mull-plugin-version:0.35.0\0more\0mull-plugin-llvm:18.1.8\0";
        assert_eq!(
            read_marker(data, MULL_VERSION_MARKER).as_deref(),
            Some("0.35.0")
        );
        assert_eq!(
            read_marker(data, LLVM_VERSION_MARKER).as_deref(),
            Some("18.1.8")
        );
    }

    #[test]
    fn missing_or_unstamped_markers() {
        assert_eq!(read_marker(b"no markers here", MULL_VERSION_MARKER), None);
        let data = b"mull-plugin-version:{STABLE_MULL_VERSION}\0";
        assert_eq!(
            read_marker(data, MULL_VERSION_MARKER).as_deref(),
            Some("dev")
        );
    }

    #[test]
    fn finds_plugins_next_to_the_executable() {
        let prefix = temp_prefix("find");
        let exe = prefix.join("bin/mull-runner");
        fs::write(&exe, b"").unwrap();
        fs::write(
            prefix.join("lib/mull-ir-frontend-20"),
            b"mull-plugin-version:0.35.0\0mull-plugin-llvm:20.1.8\0",
        )
        .unwrap();
        fs::write(prefix.join("lib/mull-ir-frontend-18"), b"old plugin").unwrap();
        fs::write(prefix.join("lib/mull-ir-frontend-x"), b"").unwrap();
        fs::write(prefix.join("lib/libfoo.so"), b"").unwrap();

        let plugins = installed_plugins(&exe);
        let names: Vec<_> = plugins
            .iter()
            .map(|p| p.path.file_name().unwrap().to_str().unwrap().to_string())
            .collect();
        assert_eq!(names, vec!["mull-ir-frontend-18", "mull-ir-frontend-20"]);
        assert_eq!(plugins[0].mull_version, None);
        assert_eq!(plugins[1].mull_version.as_deref(), Some("0.35.0"));
        assert_eq!(plugins[1].llvm_version.as_deref(), Some("20.1.8"));

        let description = describe_plugins(&plugins);
        assert!(description.contains("mull-ir-frontend-18 (mull unknown, LLVM unknown)"));
        assert!(description.contains("mull-ir-frontend-20 (mull 0.35.0, LLVM 20.1.8)"));
        let _ = fs::remove_dir_all(&prefix);
    }

    #[test]
    fn no_plugins() {
        let prefix = temp_prefix("none");
        let exe = prefix.join("bin/mull-runner");
        fs::write(&exe, b"").unwrap();
        assert!(installed_plugins(&exe).is_empty());
        assert_eq!(describe_plugins(&[]), "Plugins:\n  none found");
        let _ = fs::remove_dir_all(&prefix);
    }
}
