#include "mull/IDEDiagnostics.h"

#include "mull/MutationPoint.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/raw_ostream.h>

using namespace mull;
using namespace llvm;

void NormalIDEDiagnostics::report(mull::MutationPoint *mutationPoint,
                                  bool killed) {
  if (diagnostics == Diagnostics::None) {
    return;
  }

  if (diagnostics == Diagnostics::Survived && killed) {
    return;
  }

  if (diagnostics == Diagnostics::Killed && !killed) {
    return;
  }

  const std::string &diagnostics = mutationPoint->getDiagnostics();
  if (diagnostics.empty()) {
    return;
  }

  Instruction *instruction =
      dyn_cast<Instruction>(mutationPoint->getOriginalValue());
  if (instruction->getMetadata(0) == nullptr) {
    return;
  }

  const DebugLoc &debugLoc = instruction->getDebugLoc();

  std::string fileNameOrNil = debugLoc->getFilename().str();
  std::string lineOrNil = std::to_string(debugLoc->getLine());
  std::string columnOrNil = std::to_string(debugLoc->getColumn());

  errs() << "\n";
  errs() << fileNameOrNil << ":" << lineOrNil << ":" << columnOrNil << ": "
         << "warning: " << diagnostics << "\n";
}
