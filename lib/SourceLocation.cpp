#include "mull/SourceLocation.h"

#include "mull/Path.h"
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <unistd.h>
#include <utility>

namespace mull {

static llvm::DICompileUnit *getUnit(const llvm::DebugLoc &debugLocation) {
  llvm::DIScope *scope = debugLocation->getScope();
  while (!llvm::isa<llvm::DISubprogram>(scope) && scope != nullptr) {
    scope = scope->getScope();
  }
  return scope ? llvm::cast<llvm::DISubprogram>(scope)->getUnit() : nullptr;
}

SourceLocation::SourceLocation(std::string unitDirectory, std::string unitFilePath,
                               std::string directory, std::string filePath, size_t line,
                               size_t column)
    : unitDirectory(std::move(unitDirectory)), unitFilePath(std::move(unitFilePath)),
      directory(std::move(directory)), filePath(std::move(filePath)), line(line), column(column) {}

SourceLocation SourceLocation::nullSourceLocation() {
  return SourceLocation(std::string(), std::string(), std::string(), std::string(), 0, 0);
}

SourceLocation SourceLocation::locationFromInstruction(const llvm::Instruction *instruction) {
  if (instruction->getMetadata(0) == nullptr) {
    return nullSourceLocation();
  }

  const llvm::DebugLoc &debugInfo = instruction->getDebugLoc();

  std::string directory = debugInfo->getDirectory().str();
  std::string filePath = debugInfo->getFilename().str();
  int line = debugInfo->getLine();
  int column = debugInfo->getColumn();

  filePath = absoluteFilePath(directory, filePath);

  std::string unitDirectory;
  std::string unitFilePath;
  llvm::DICompileUnit *unit = getUnit(debugInfo);
  if (unit) {
    unitDirectory = unit->getDirectory();
    unitFilePath = unit->getFilename();
  }

  unitFilePath = absoluteFilePath(unitDirectory, unitFilePath);

  return SourceLocation(unitDirectory, unitFilePath, directory, filePath, line, column);
}

SourceLocation SourceLocation::locationFromFunction(const llvm::Function *function) {
  if (function->getMetadata(0) == nullptr) {
    return nullSourceLocation();
  }

  auto debugInfo = llvm::dyn_cast<llvm::DISubprogram>(function->getMetadata(0));

  std::string unitDirectory;
  std::string unitFilePath;
  if (llvm::DICompileUnit *unit = debugInfo->getUnit()) {
    unitDirectory = unit->getDirectory();
    unitFilePath = unit->getFilename();
  }

  std::string directory = debugInfo->getDirectory().str();
  std::string filePath = debugInfo->getFilename().str();
  int line = debugInfo->getLine();
  int column = 0;

  filePath = absoluteFilePath(directory, filePath);
  unitFilePath = absoluteFilePath(unitDirectory, unitFilePath);

  return SourceLocation(unitDirectory, unitFilePath, directory, filePath, line, column);
}

bool SourceLocation::isNull() const {
  return (directory.empty() && filePath.empty() && line == 0 && column == 0) ||
         // this case happened when compiled with '-Og', please look at:
         // https://github.com/mull-project/mull/issues/519
         // https://github.com/mull-project/mull/issues/520
         (line == 0 && column == 0);
}

bool SourceLocation::canRead() const {
  if (access(filePath.c_str(), R_OK) != 0) {
    return false;
  }
  return true;
}

} // namespace mull
