#pragma once

#include <string>

namespace llvm {
class Function;
}

namespace mull {

class FunctionFilter {
public:
  virtual bool shouldSkip(llvm::Function *function) = 0;
  virtual std::string name() = 0;
};

} // namespace mull
