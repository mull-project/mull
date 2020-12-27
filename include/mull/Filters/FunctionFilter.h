#pragma once

#include "mull/Filters/Filter.h"

#include <string>

namespace llvm {
class Function;
}

namespace mull {

class FunctionFilter : virtual public Filter {
public:
  virtual bool shouldSkip(llvm::Function *function) = 0;
  virtual std::string name() = 0;
  ~FunctionFilter() override = default;
};

} // namespace mull
