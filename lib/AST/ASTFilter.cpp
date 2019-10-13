#include "mull/AST/ASTFilter.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Module.h>

#include <iostream>

using namespace mull;

ASTFilter::ASTFilter(const ASTMutations &astMutations,
                     const FilePathFilter &filePathFilter)
    : astMutations(astMutations), filePathFilter(filePathFilter) {}

bool ASTFilter::validMutation(llvm::Instruction &instruction) const {
  const llvm::DebugLoc &debugInfo = instruction.getDebugLoc();
  if (!debugInfo) {
    return false;
  }

  int line = debugInfo.getLine();
  int column = debugInfo.getCol();

  const llvm::DILocation *diLocation = debugInfo.get();
  std::string fileName = diLocation->getFilename();

  if (filePathFilter.shouldSkip(fileName)) {
    return false;
  }

  if (line <= 0 || column <= 0) {
    // TODO: STAN What is here?
    return false;
  }

  bool locationExists = astMutations.locationExists(fileName, line, column);

  return locationExists;
}
