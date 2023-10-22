#include "DynamicLibraries.h"
#include "ObjectFile.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ELFTypes.h>
#include <llvm/Object/MachO.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <unistd.h>
#ifdef __APPLE__
#include <dlfcn.h>
#endif

using namespace llvm::object;
using namespace std::string_literals;

static llvm::StringRef getSectionName(const llvm::object::SectionRef &section) {
  llvm::Expected<llvm::StringRef> name = section.getName();
  if (!name) {
    return {};
  }
  return name.get();
}

static llvm::StringRef getSectionContent(const llvm::object::SectionRef &section) {
  llvm::Expected<llvm::StringRef> content = section.getContents();
  if (!content) {
    return {};
  }
  return content.get();
}

template <typename T>
void librariesFromElf(const ELFObjectFile<T> &file, std::vector<std::string> &libraries) {
  SectionRef nullSection;
  section_iterator stringTableIterator(nullSection);
  section_iterator dynamicSectionIterator(nullSection);

  for (auto it = file.section_begin(); it != file.section_end(); ++it) {
    ELFSectionRef elfSection(*it);
    llvm::StringRef name = getSectionName(elfSection);
    // FIXME: the dynamic string table should be taken based on .dynamic
    // section's sh_link value
    if (elfSection.getType() == llvm::ELF::SHT_STRTAB && name.equals(".dynstr")) {
      stringTableIterator = it;
    }
    if (elfSection.getType() == llvm::ELF::SHT_DYNAMIC) {
      dynamicSectionIterator = it;
    }
  }

  assert((*stringTableIterator) != nullSection && "Could not find string table");
  assert((*dynamicSectionIterator) != nullSection && "Could not find .dynamic section");

  llvm::StringRef stringTable = getSectionContent(*stringTableIterator);
  llvm::StringRef dynamicSection = getSectionContent(*dynamicSectionIterator);

  Elf_Dyn_Base<T> dynamicEntry{};
  uint64_t sectionSize = dynamicSectionIterator->getSize();
  size_t entrySize = sizeof(dynamicEntry);
  assert((sectionSize % entrySize) == 0 &&
         "Number of entries should be aligned with the size of the section");
  size_t entriesCount = sectionSize / entrySize;

  for (size_t i = 0; i < entriesCount; i++) {
    auto offset = i * entrySize;
    memcpy(&dynamicEntry, dynamicSection.data() + offset, entrySize);
    if (dynamicEntry.d_tag == llvm::ELF::DT_NEEDED) {
      libraries.emplace_back(stringTable.data() + dynamicEntry.d_un.d_val);
    }
  }
}

void librariesFromMachO(const MachOObjectFile &file, std::vector<std::string> &libraries) {
  for (auto &loadCommand : file.load_commands()) {
    if (loadCommand.C.cmd != llvm::MachO::LC_LOAD_DYLIB) {
      continue;
    }
    auto dylib = file.getDylibIDLoadCommand(loadCommand);
    libraries.emplace_back(loadCommand.Ptr + dylib.dylib.name);
  }
}

static std::string resolveLibraryPath(const std::string &library,
                                      const std::vector<std::string> &librarySearchPaths) {
  if (llvm::sys::path::is_absolute(library)) {
    return library;
  }

  for (auto &searchPath : librarySearchPaths) {
    std::string fullPath = searchPath + "/" + library;
    if (llvm::sys::fs::exists(fullPath)) {
      return fullPath;
    }
  }

  return library;
}

void mull::resolveLibraries(mull::Diagnostics &diagnostics,
                            std::vector<std::string> &resolvedLibraries,
                            const std::vector<std::string> &libraries,
                            const std::vector<std::string> &librarySearchPaths) {
  resolvedLibraries.reserve(resolvedLibraries.size() + libraries.size());
  for (auto &library : libraries) {
    auto libraryPath = resolveLibraryPath(library, librarySearchPaths);
    if (llvm::sys::fs::exists(libraryPath)) {
      resolvedLibraries.push_back(std::move(libraryPath));
    } else {
      /// on macOS, system dylibs are not present on disk, but still valid and available
      /// through the dyld cache.
      /// In this case, we don't care about the libraries (system libraries won't have mutants)
      /// but we don't want to show useless warnings
      bool showWarning = true;
#ifdef __APPLE__
      void *handle = dlopen(library.c_str(), RTLD_LAZY);
      if (handle != nullptr) {
        /// dyld cache
        showWarning = false;
      }
#endif
      if (showWarning) {
        diagnostics.warning("Could not find dynamic library: "s + library);
      }
    }
  }
}

std::vector<std::string> mull::getDynamicLibraryDependencies(mull::Diagnostics &diagnostics,
                                                             const std::string &executablePath) {
  std::vector<std::string> libraries;
  auto [buffer, objectFile] = mull::loadObjectFile(diagnostics, executablePath);
  if (!objectFile) {
    return libraries;
  }
  if (objectFile->isMachO()) {
    auto machOFile = llvm::dyn_cast<MachOObjectFile>(objectFile.get());
    librariesFromMachO(*machOFile, libraries);
  } else if (objectFile->isELF()) {
    ObjectFile *elfPtr = objectFile.get();

    if (auto elf32LEFile = llvm::dyn_cast<ELF32LEObjectFile>(elfPtr)) {
      librariesFromElf(*elf32LEFile, libraries);
    } else if (auto elf32BEFile = llvm::dyn_cast<ELF32BEObjectFile>(elfPtr)) {
      librariesFromElf(*elf32BEFile, libraries);
    } else if (auto elf64LEFile = llvm::dyn_cast<ELF64LEObjectFile>(elfPtr)) {
      librariesFromElf(*elf64LEFile, libraries);
    } else if (auto elf64BEFile = llvm::dyn_cast<ELF64BEObjectFile>(elfPtr)) {
      librariesFromElf(*elf64BEFile, libraries);
    }
  }
  return libraries;
}

bool mull::hasCoverage(mull::Diagnostics &diagnostics, const std::string &path) {
  auto [buffer, objectFile] = loadObjectFile(diagnostics, path);
  if (!objectFile) {
    return false;
  }

  for (auto &section : objectFile->sections()) {
    if (getSectionName(section).endswith("llvm_covmap")) {
      return true;
    }
  }

  return false;
}
