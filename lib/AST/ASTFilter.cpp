#include "mull/AST/ASTFilter.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Module.h>

#include <iostream>
#include <llvm/IR/Instructions.h>

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

  if (line == 17 && column == 10) {

  }


//  instruction.getModule()->print(llvm::errs(), nullptr, false, true);

  bool locationExists = astMutations.locationExists(fileName, line, column);

  if (locationExists) {
    llvm::errs() << "\n";
    instruction.print(llvm::errs(), true);
    llvm::errs() << "\nline/col: " << line << " " << column << "\n";
    llvm::errs() << "\n";
  }

  return locationExists;
}

bool ASTFilter::validScalarMutation(llvm::Instruction &instruction) const {
  const llvm::DebugLoc &debugInfo = instruction.getDebugLoc();
  if (!debugInfo) {
    return false;
  }

  /// TODO: we might want to reconsider this because some mutations can be
  /// part of this instruction.
  if (llvm::dyn_cast<llvm::GetElementPtrInst>(&instruction)) {
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

  if (locationExists) {
    llvm::errs() << "\n";
    instruction.print(llvm::errs(), true);
    llvm::errs() << "\nline/col: " << line << " " << column << "\n";
    llvm::errs() << "\n";
  }

  return locationExists;
}
