#include "mull/Filters/NoDebugInfoFilter.h"

#include "mull/MutationPoint.h"

using namespace mull;

bool NoDebugInfoFilter::shouldSkip(MutationPoint *point) {
  return point->getSourceLocation().isNull();
}

bool NoDebugInfoFilter::shouldSkip(llvm::Function *function) {
  return SourceLocation::locationFromFunction(function).isNull();
}

bool NoDebugInfoFilter::shouldSkip(llvm::Instruction *instruction) {
  return SourceLocation::locationFromInstruction(instruction).isNull();
}

std::string NoDebugInfoFilter::name() { return "no debug info"; }
