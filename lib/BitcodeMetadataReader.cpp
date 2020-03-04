#include "mull/BitcodeMetadataReader.h"

#include "mull/Bitcode.h"
#include "mull/Path.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Module.h>

std::map<std::string, std::string> mull::BitcodeMetadataReader::getCompilationDatabase(
    std::vector<std::unique_ptr<mull::Bitcode>> &bitcode) {
  std::map<std::string, std::string> bitcodeCompilationFlags;
  if (bitcode.empty()) {
    return bitcodeCompilationFlags;
  }
  for (auto &bitcodeModule: bitcode) {
    llvm::Module *module = bitcodeModule->getModule();

    for (llvm::DICompileUnit *unit : module->debug_compile_units()) {
      llvm::StringRef directory = unit->getDirectory();
      llvm::StringRef fileName = unit->getFilename();
      std::string unitFullPath = absoluteFilePath(directory, fileName);

      llvm::StringRef unitFlags = unit->getFlags();

      if (!unitFlags.empty()) {
        bitcodeCompilationFlags[unitFullPath] = unitFlags.str();
      }
    }
  }
  return bitcodeCompilationFlags;
}
