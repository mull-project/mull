use lddtree::DependencyAnalyzer;
use mull_core::{diag_debug, diag_warning, diagnostics::MullDiagnostics};
#[cfg(target_os = "macos")]
use object::Object;
use std::collections::BTreeSet;
use std::path::PathBuf;

/// Resolve direct and transitive dependencies like the dynamic loader; `search_paths` are tried last.
pub fn find_dynamic_libraries(
    diag: &MullDiagnostics,
    path: &str,
    search_paths: &[String],
) -> Vec<String> {
    diag_debug!(diag, "Getting dynamic library dependencies for '{}'", path);

    let analyzer = DependencyAnalyzer::default()
        .library_paths(search_paths.iter().map(PathBuf::from).collect());
    let tree = match analyzer.analyze(path) {
        Ok(t) => t,
        Err(e) => {
            diag_warning!(diag, "Cannot read dynamic libraries of '{}': {}", path, e);
            return Vec::new();
        }
    };

    let mut libraries: Vec<_> = tree.libraries.into_values().collect();
    libraries.sort_by(|a, b| a.name.cmp(&b.name));

    let mut resolved = BTreeSet::new();
    for lib in libraries {
        match lib.realpath {
            Some(realpath) => {
                diag_debug!(diag, "  dependency: {} -> {}", lib.name, realpath.display());
                resolved.insert(realpath.to_string_lossy().into_owned());
            }
            None if is_in_dyld_cache(&lib.name) => {}
            None => diag_warning!(diag, "Could not find dynamic library: {}", lib.name),
        }
    }
    resolved.into_iter().collect()
}

#[cfg(target_os = "macos")]
pub fn maybe_extract_fat_slice<'a>(data: &'a [u8], diag: &MullDiagnostics) -> &'a [u8] {
    use object::read::macho::{FatArch, MachOFatFile32, MachOFatFile64};

    #[cfg(target_arch = "aarch64")]
    let current_arch = object::Architecture::Aarch64;
    #[cfg(target_arch = "x86_64")]
    let current_arch = object::Architecture::X86_64;

    fn find_slice<'a, F: FatArch>(
        data: &'a [u8],
        arches: &[F],
        current_arch: object::Architecture,
    ) -> Option<(&'a [u8], bool)> {
        // Try current arch first
        for arch in arches {
            if let Ok(slice) = arch.data(data)
                && let Ok(obj) = object::File::parse(slice)
                && obj.architecture() == current_arch
            {
                return Some((slice, true));
            }
        }
        // Fall back to first
        arches
            .first()
            .and_then(|a| a.data(data).ok())
            .map(|s| (s, false))
    }

    if let Ok(fat) = MachOFatFile32::parse(data)
        && let Some((slice, is_current)) = find_slice(data, fat.arches(), current_arch)
    {
        if is_current {
            diag_debug!(diag, "Using {:?} slice from fat binary", current_arch);
        } else {
            diag_debug!(diag, "Using first slice from fat binary");
        }
        return slice;
    }
    if let Ok(fat) = MachOFatFile64::parse(data)
        && let Some((slice, is_current)) = find_slice(data, fat.arches(), current_arch)
    {
        if is_current {
            diag_debug!(diag, "Using {:?} slice from fat binary", current_arch);
        } else {
            diag_debug!(diag, "Using first slice from fat binary");
        }
        return slice;
    }

    data
}

#[cfg(not(target_os = "macos"))]
pub fn maybe_extract_fat_slice<'a>(data: &'a [u8], _diag: &MullDiagnostics) -> &'a [u8] {
    data
}

#[cfg(target_os = "macos")]
fn is_in_dyld_cache(library: &str) -> bool {
    use std::ffi::{c_char, CString};
    unsafe extern "C" {
        fn _dyld_shared_cache_contains_path(path: *const c_char) -> bool;
    }
    let Ok(c_lib) = CString::new(library) else {
        return false;
    };
    unsafe { _dyld_shared_cache_contains_path(c_lib.as_ptr()) }
}

#[cfg(not(target_os = "macos"))]
fn is_in_dyld_cache(_library: &str) -> bool {
    false
}
