#include "mull/BitcodeMetadataReader.h"

#include "mull/Bitcode.h"
#include "mull/Path.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Module.h>

using namespace mull;

std::unordered_map<std::string, std::string> BitcodeMetadataReader::getCompilationDatabase(
    std::vector<std::unique_ptr<mull::Bitcode>> &bitcode) {
  std::unordered_map<std::string, std::string> bitcodeCompilationFlags;
  if (bitcode.empty()) {
    return bitcodeCompilationFlags;
  }
  for (auto &bitcodeModule : bitcode) {
    llvm::Module *module = bitcodeModule->getModule();

    for (llvm::DICompileUnit *unit : module->debug_compile_units()) {
      llvm::StringRef directory = unit->getDirectory();
      llvm::StringRef fileName = unit->getFilename();
      std::string unitFullPath = absoluteFilePath(directory.str(), fileName.str());

      llvm::StringRef unitFlags = unit->getFlags();

      if (!unitFlags.empty()) {
        bitcodeCompilationFlags[unitFullPath] = unitFlags.str();
      }
    }
  }
  return bitcodeCompilationFlags;
}

std::unordered_map<std::string, std::string>
BitcodeMetadataReader::getCompilationDatabase(llvm::Module &module) {
  std::unordered_map<std::string, std::string> bitcodeCompilationFlags;

  for (llvm::DICompileUnit *unit : module.debug_compile_units()) {
    llvm::StringRef directory = unit->getDirectory();
    llvm::StringRef fileName = unit->getFilename();
    std::string unitFullPath = absoluteFilePath(directory.str(), fileName.str());

    llvm::StringRef unitFlags = unit->getFlags();

    if (!unitFlags.empty()) {
      bitcodeCompilationFlags[unitFullPath] = unitFlags.str();
    }
  }
  return bitcodeCompilationFlags;
}
