#pragma once

#include "mull/Filters/FunctionFilter.h"

namespace mull {

class BlockAddressFunctionFilter : public FunctionFilter {
public:
  bool shouldSkip(llvm::Function *function) override;
  std::string name() override;
  ~BlockAddressFunctionFilter() override = default;
};
} // namespace mull
