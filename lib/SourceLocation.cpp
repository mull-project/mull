#include "mull/SourceLocation.h"

#include <string>

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/Path.h>

namespace mull {

SourceLocation::SourceLocation(const std::string &directory,
                               const std::string &filePath, int line,
                               int column)
    : directory(directory), filePath(filePath), line(line), column(column) {}

const SourceLocation SourceLocation::nullSourceLocation() {
  return SourceLocation(std::string(), std::string(), 0, 0);
}

const SourceLocation SourceLocation::sourceLocationFromInstruction(
    const llvm::Instruction *instruction) {
  if (instruction->getMetadata(0) == nullptr) {
    return nullSourceLocation();
  }

  const llvm::DebugLoc &debugInfo = instruction->getDebugLoc();

  std::string directory = debugInfo->getDirectory();
  std::string filePath = debugInfo->getFilename();
  int line = debugInfo->getLine();
  int column = debugInfo->getColumn();

  if (!llvm::sys::path::is_absolute(filePath)) {
    filePath = directory + llvm::sys::path::get_separator().str() + filePath;
  }

  return SourceLocation(directory, filePath, line, column);
}

const SourceLocation
SourceLocation::sourceLocationFromFunction(const llvm::Function *function) {
  if (function->getMetadata(0) == nullptr) {
    return nullSourceLocation();
  }

  auto debugInfo = llvm::dyn_cast<llvm::DISubprogram>(function->getMetadata(0));

  std::string directory = debugInfo->getDirectory();
  std::string filePath = debugInfo->getFilename();
  int line = debugInfo->getLine();
  int column = 0;

  if (!llvm::sys::path::is_absolute(filePath)) {
    filePath = directory + llvm::sys::path::get_separator().str() + filePath;
  }

  return SourceLocation(directory, filePath, line, column);
}

bool SourceLocation::isNull() const {
  return directory == "" && filePath == "" && line == 0 && column == 0;
}

} // namespace mull
