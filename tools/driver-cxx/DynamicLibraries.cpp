#include "DynamicLibraries.h"

#include <llvm/BinaryFormat/Magic.h>
#include <llvm/BinaryFormat/MachO.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/MachO.h>
#include <llvm/Support/MemoryBuffer.h>

void librariesFromMachO(const llvm::object::MachOObjectFile &file,
                        std::vector<std::string> &libraries) {
  for (auto &loadCommand : file.load_commands()) {
    if (loadCommand.C.cmd != llvm::MachO::LC_LOAD_DYLIB) {
      continue;
    }

    auto dylib = file.getDylibIDLoadCommand(loadCommand);
    libraries.emplace_back(loadCommand.Ptr + dylib.dylib.name);
  }
}

std::vector<std::string>
mull::findDynamicLibraries(const std::string &executablePath) {
  std::vector<std::string> libraries;

  auto bufferOr = llvm::MemoryBuffer::getFile(executablePath);
  std::unique_ptr<llvm::MemoryBuffer> buffer(std::move(bufferOr.get()));

  auto symbolicOr = llvm::object::SymbolicFile::createSymbolicFile(
      buffer->getMemBufferRef(), llvm::file_magic::unknown, nullptr);

  std::unique_ptr<llvm::object::SymbolicFile> symbolicFile(
      std::move(symbolicOr.get()));

  if (symbolicFile->isMachO()) {
    auto machOFile =
        llvm::dyn_cast<llvm::object::MachOObjectFile>(symbolicFile.get());
    librariesFromMachO(*machOFile, libraries);
  }

  return libraries;
}
