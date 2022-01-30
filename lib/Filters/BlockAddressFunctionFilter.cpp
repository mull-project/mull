#include "mull/Filters/BlockAddressFunctionFilter.h"
#include <llvm/IR/Function.h>

using namespace mull;

bool BlockAddressFunctionFilter::shouldSkip(llvm::Function *function) {
  for (auto &bb : *function) {
    if (bb.hasAddressTaken()) {
      return true;
    }
  }
  return false;
}

std::string BlockAddressFunctionFilter::name() {
  return "block address (jump tables)";
}
