#pragma once

#include "mull/Filters/FunctionFilter.h"

namespace mull {

class VariadicFunctionFilter : public FunctionFilter {
public:
  bool shouldSkip(llvm::Function *function) override;
  std::string name() override;
  ~VariadicFunctionFilter() override = default;
};
} // namespace mull
