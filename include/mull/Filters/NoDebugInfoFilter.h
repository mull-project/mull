#pragma once

#include "MutationFilter.h"
#include "mull/Filters/FunctionFilter.h"

namespace mull {

class NoDebugInfoFilter : public MutationFilter, public FunctionFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(llvm::Function *function) override;
  std::string name() override;

private:
};
} // namespace mull