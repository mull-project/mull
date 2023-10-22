#include "ObjectFile.h"

#include <llvm/Object/ELF.h>
#include <llvm/Object/MachO.h>
#include <llvm/Object/MachOUniversal.h>
#include <llvm/Object/SymbolicFile.h>

using namespace std::string_literals;

static std::string getCurrentArch() {
#ifdef __APPLE__
#ifdef __ARM_ARCH
  return "arm64";
#else
  return "x86_64";
#endif
#endif
  return "";
}

static std::unique_ptr<llvm::object::ObjectFile>
getFirstObject(mull::Diagnostics &diagnostics, const std::string &executablePath,
               llvm::object::MachOUniversalBinary &binary) {
  assert(!binary.objects().empty());
  auto object = *binary.objects().begin();
  auto objectOr = object.getAsObjectFile();
  if (!objectOr) {
    diagnostics.error("Cannot create ObjectFile from: "s + executablePath + ": " +
                      llvm::toString(objectOr.takeError()));
    return {};
  }
  return std::move(objectOr.get());
}

static std::unique_ptr<llvm::object::ObjectFile>
getObjectFile(mull::Diagnostics &diagnostics, const std::string &executablePath,
              std::unique_ptr<llvm::MemoryBuffer> &buffer) {
  auto binaryOr = llvm::object::createBinary(buffer->getMemBufferRef());
  if (!binaryOr) {
    diagnostics.error("Cannot create binary object from: "s + executablePath + ": " +
                      llvm::toString(binaryOr.takeError()));
    return {};
  }
  std::unique_ptr<llvm::object::Binary> binary(std::move(binaryOr.get()));
  if (auto universalBinary = llvm::dyn_cast<llvm::object::MachOUniversalBinary>(binary.get())) {
    if (universalBinary->objects().empty()) {
      diagnostics.error("Universal binary is empty: "s + executablePath);
      return {};
    }
    auto currentArch = getCurrentArch();
    if (currentArch.empty()) {
      diagnostics.info("Cannot determine current arch, trying the first arch: "s +
                       universalBinary->objects().begin()->getArchFlagName());
      return getFirstObject(diagnostics, executablePath, *universalBinary);
    }
    auto objectOr = universalBinary->getMachOObjectForArch(currentArch);
    if (!objectOr) {
      auto error = objectOr.takeError();
      diagnostics.info("Cannot get current arch ('"s + currentArch + "') from universal binary: " +
                       llvm::toString(std::move(error)) + ".\nTrying the first arch: "s +
                       universalBinary->objects().begin()->getArchFlagName());
      return getFirstObject(diagnostics, executablePath, *universalBinary);
    }
    return std::move(objectOr.get());
  }
  if (binary->isObject()) {
    auto objectFile = llvm::dyn_cast<llvm::object::ObjectFile>(binary.release());
    return std::unique_ptr<llvm::object::ObjectFile>(objectFile);
  }
  diagnostics.warning("Unknown binary type: "s + executablePath + ": " +
                      std::to_string(binary->getType()));
  return {};
}

mull::OwnedObjectFile mull::loadObjectFile(mull::Diagnostics &diagnostics,
                                           const std::string &executablePath) {
  auto bufferOr = llvm::MemoryBuffer::getFile(executablePath);
  if (!bufferOr) {
    diagnostics.error("Cannot open executable: "s + executablePath);
    return {};
  }
  std::unique_ptr<llvm::MemoryBuffer> buffer(std::move(bufferOr.get()));
  auto object = getObjectFile(diagnostics, executablePath, buffer);
  return { std::move(buffer), std::move(object) };
}
