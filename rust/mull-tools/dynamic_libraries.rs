use mull_core::{diag_debug, diag_warning, diagnostics::MullDiagnostics};
use object::read::elf::{Dyn, FileHeader, SectionHeader};
use object::Endianness;
use std::fs;
use std::path::Path;

/// Get dynamic library dependencies from an executable.
pub fn get_dynamic_library_dependencies(diag: &MullDiagnostics, path: &str) -> Vec<String> {
    diag_debug!(diag, "Getting dynamic library dependencies for '{}'", path);

    let data = match fs::read(path) {
        Ok(d) => d,
        Err(e) => {
            diag_warning!(diag, "Cannot read '{}': {}", path, e);
            return Vec::new();
        }
    };

    let data = maybe_extract_fat_slice(&data, diag);

    let file = match object::File::parse(data) {
        Ok(f) => f,
        Err(e) => {
            diag_warning!(diag, "Cannot parse '{}': {}", path, e);
            return Vec::new();
        }
    };

    let mut libraries = Vec::new();

    match &file {
        object::File::MachO32(macho) => libraries_from_macho(macho, &mut libraries),
        object::File::MachO64(macho) => libraries_from_macho(macho, &mut libraries),
        object::File::Elf32(elf) => libraries_from_elf(elf, &mut libraries),
        object::File::Elf64(elf) => libraries_from_elf(elf, &mut libraries),
        _ => {}
    }

    for lib in &libraries {
        diag_debug!(diag, "  dependency: {}", lib);
    }

    libraries
}

fn libraries_from_elf<Elf: FileHeader<Endian = Endianness>>(
    elf: &object::read::elf::ElfFile<'_, Elf>,
    libraries: &mut Vec<String>,
) {
    use object::read::StringTable;

    let endian = elf.endian();
    let sections = elf.elf_section_table();
    let data = elf.data();

    // Find the dynamic section and get DT_NEEDED entries
    for section in sections.iter() {
        if section.sh_type(endian) != object::elf::SHT_DYNAMIC {
            continue;
        }

        // Get the dynamic entries and linked string table
        let Ok(Some((dynamic_entries, link))) = section.dynamic(endian, data) else {
            continue;
        };

        // Get the string table from the linked section
        let Ok(strtab_section) = sections.section(link) else {
            continue;
        };
        let Ok(strtab_data) = strtab_section.data(endian, data) else {
            continue;
        };
        let strings = StringTable::new(strtab_data, 0, strtab_data.len() as u64);

        // Extract DT_NEEDED entries
        for dyn_entry in dynamic_entries {
            let tag = dyn_entry.tag32(endian);
            if tag == Some(object::elf::DT_NEEDED) {
                if let Ok(name) = dyn_entry.string(endian, strings) {
                    if let Ok(s) = std::str::from_utf8(name) {
                        libraries.push(s.to_string());
                    }
                }
            } else if tag == Some(object::elf::DT_NULL) {
                break;
            }
        }
        break; // Only process first dynamic section
    }
}

fn libraries_from_macho<Mach: object::read::macho::MachHeader>(
    macho: &object::read::macho::MachOFile<'_, Mach>,
    libraries: &mut Vec<String>,
) {
    use object::read::macho::LoadCommandVariant;

    let endian = macho.endian();
    let header = macho.macho_header();
    let data = macho.data();

    let Ok(commands) = header.load_commands(endian, data, 0) else {
        return;
    };

    for cmd in commands.flatten() {
        if let Ok(LoadCommandVariant::Dylib(dylib)) = cmd.variant() {
            if let Ok(name) = cmd.string(endian, dylib.dylib.name) {
                if let Ok(s) = std::str::from_utf8(name) {
                    libraries.push(s.to_string());
                }
            }
        }
    }
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
            if let Ok(slice) = arch.data(data) {
                if let Ok(obj) = object::File::parse(slice) {
                    if obj.architecture() == current_arch {
                        return Some((slice, true));
                    }
                }
            }
        }
        // Fall back to first
        arches
            .first()
            .and_then(|a| a.data(data).ok())
            .map(|s| (s, false))
    }

    if let Ok(fat) = MachOFatFile32::parse(data) {
        if let Some((slice, is_current)) = find_slice(data, fat.arches(), current_arch) {
            if is_current {
                diag_debug!(diag, "Using {:?} slice from fat binary", current_arch);
            } else {
                diag_debug!(diag, "Using first slice from fat binary");
            }
            return slice;
        }
    }
    if let Ok(fat) = MachOFatFile64::parse(data) {
        if let Some((slice, is_current)) = find_slice(data, fat.arches(), current_arch) {
            if is_current {
                diag_debug!(diag, "Using {:?} slice from fat binary", current_arch);
            } else {
                diag_debug!(diag, "Using first slice from fat binary");
            }
            return slice;
        }
    }

    data
}

#[cfg(not(target_os = "macos"))]
pub fn maybe_extract_fat_slice<'a>(data: &'a [u8], _diag: &MullDiagnostics) -> &'a [u8] {
    data
}

/// Resolve library paths using search paths.
pub fn resolve_libraries(
    diag: &MullDiagnostics,
    libraries: &[String],
    search_paths: &[String],
) -> Vec<String> {
    libraries
        .iter()
        .filter_map(|lib| {
            let path = resolve_library_path(lib, search_paths);
            if Path::new(&path).exists() {
                Some(path)
            } else {
                #[cfg(target_os = "macos")]
                if is_in_dyld_cache(lib) {
                    return None; // Skip dyld cache libraries silently
                }
                diag_warning!(diag, "Could not find dynamic library: {}", lib);
                None
            }
        })
        .collect()
}

fn resolve_library_path(library: &str, search_paths: &[String]) -> String {
    if Path::new(library).is_absolute() {
        return library.to_string();
    }

    for search_path in search_paths {
        let full_path = Path::new(search_path).join(library);
        if full_path.exists() {
            return full_path.to_string_lossy().into_owned();
        }
    }

    library.to_string()
}

#[cfg(target_os = "macos")]
fn is_in_dyld_cache(library: &str) -> bool {
    use std::ffi::CString;
    let Ok(c_lib) = CString::new(library) else {
        return false;
    };
    unsafe {
        let handle = libc::dlopen(c_lib.as_ptr(), libc::RTLD_LAZY);
        if !handle.is_null() {
            libc::dlclose(handle);
            true
        } else {
            false
        }
    }
}
