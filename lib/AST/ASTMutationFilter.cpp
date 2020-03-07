#include "mull/AST/ASTMutationFilter.h"

#include "mull/MutationPoint.h"

#include <llvm/IR/Instruction.h>
#include <llvm/Support/FileSystem.h>

using namespace mull;

ASTMutationFilter::ASTMutationFilter(Diagnostics &diagnostics, ASTStorage &astStorage)
    : diagnostics(diagnostics), astStorage(astStorage) {}

bool ASTMutationFilter::shouldSkip(MutationPoint *point) {
  llvm::Instruction *instruction = llvm::dyn_cast<llvm::Instruction>(point->getOriginalValue());
  assert(instruction);

  const mull::MutatorKind mutatorKind = point->getMutator()->mutatorKind();

  const llvm::DebugLoc &debugInfo = instruction->getDebugLoc();
  if (!debugInfo) {
    return false;
  }

  int line = debugInfo.getLine();
  int column = debugInfo.getCol();

  if (line <= 0 || column <= 0) {
    assert(0 && "ASTMutationFilter: Unknown edge case.");
  }

  mull::SourceLocation sourceLocation = SourceLocation::locationFromInstruction(instruction);

  const std::string fullSourcePath = sourceLocation.filePath;
  assert(llvm::sys::fs::is_regular_file(fullSourcePath));

  bool astMutationExists = astStorage.mutationExists(fullSourcePath, mutatorKind, line, column);
  if (astMutationExists) {
    std::string description = MutationKindToString(mutatorKind);
    diagnostics.debug(std::string("ASTMutationFilter: whitelisting mutation \"") + description +
                      "\": " + sourceLocation.filePath + ":" + std::to_string(line) + ":" +
                      std::to_string(column));
  }

  return astMutationExists == false;
}

std::string mull::ASTMutationFilter::name() {
  return "AST mutation filter";
}
