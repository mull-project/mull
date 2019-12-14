#include "mull/IDEDiagnostics.h"

#include "mull/MutationPoint.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/raw_ostream.h>

using namespace mull;
using namespace llvm;

void NormalIDEDiagnostics::report(mull::MutationPoint *mutationPoint, bool killed) {
  if (diagnostics == Diagnostics::None) {
    return;
  }

  if (diagnostics == Diagnostics::Survived && killed) {
    return;
  }

  if (diagnostics == Diagnostics::Killed && !killed) {
    return;
  }

  const std::string &message = mutationPoint->getDiagnostics();
  if (message.empty()) {
    return;
  }

  SourceLocation sourceLocation = mutationPoint->getSourceLocation();
  if (sourceLocation.isNull()) {
    return;
  }

  errs() << "\n"
         << sourceLocation.filePath << ":" << sourceLocation.line << ":" << sourceLocation.column
         << ": warning: " << message << "\n";
}
