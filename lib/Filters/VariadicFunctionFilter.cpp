#include "mull/Filters/VariadicFunctionFilter.h"
#include <llvm/IR/Function.h>

using namespace mull;

bool VariadicFunctionFilter::shouldSkip(llvm::Function *function) {
  return function->isVarArg();
}

std::string VariadicFunctionFilter::name() {
  return "variadic functions";
}
