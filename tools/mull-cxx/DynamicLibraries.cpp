#include "DynamicLibraries.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <LLVMCompatibility.h>
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ELFTypes.h>
#include <llvm/Object/MachO.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <unistd.h>

using namespace llvm::object;

template <typename T>
void librariesFromElf(const ELFObjectFile<T> &file, std::vector<std::string> &libraries) {
  SectionRef nullSection;
  section_iterator stringTableIterator(nullSection);
  section_iterator dynamicSectionIterator(nullSection);

  for (auto it = file.section_begin(); it != file.section_end(); ++it) {
    ELFSectionRef elfSection(*it);
    llvm::StringRef name;
    elfSection.getName(name);
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

  llvm::StringRef stringTable = llvm_compat::getSectionContent(*stringTableIterator);
  llvm::StringRef dynamicSection = llvm_compat::getSectionContent(*dynamicSectionIterator);

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

std::vector<std::string> mull::findDynamicLibraries(mull::Diagnostics &diagnostics,
                                                    const std::string &executablePath,
                                                    std::vector<std::string> &librarySearchPaths) {
  std::vector<std::string> libraries;

  auto bufferOr = llvm::MemoryBuffer::getFile(executablePath);
  if (!bufferOr) {
    diagnostics.error(std::string("Cannot open executable: ") + executablePath);
  }
  std::unique_ptr<llvm::MemoryBuffer> buffer(std::move(bufferOr.get()));

  auto symbolicOr = SymbolicFile::createSymbolicFile(buffer->getMemBufferRef());
  if (!symbolicOr) {
    diagnostics.error(std::string("Cannot create symbolic file from: ") + executablePath);
  }

  std::unique_ptr<SymbolicFile> symbolicFile(std::move(symbolicOr.get()));

  if (symbolicFile->isMachO()) {
    auto machOFile = llvm::dyn_cast<MachOObjectFile>(symbolicFile.get());
    librariesFromMachO(*machOFile, libraries);
  } else if (symbolicFile->isELF()) {
    SymbolicFile *elfPtr = symbolicFile.get();

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

  std::vector<std::string> resolvedLibraries;
  for (auto &library : libraries) {
    auto libraryPath = resolveLibraryPath(library, librarySearchPaths);
    if (llvm::sys::fs::exists(libraryPath)) {
      resolvedLibraries.push_back(libraryPath);
    } else {
      diagnostics.warning(std::string("Could not find dynamic library: ") + library);
    }
  }

  return resolvedLibraries;
}
