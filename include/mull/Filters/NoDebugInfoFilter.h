#pragma once

#include "mull/Filters/FunctionFilter.h"
#include "mull/Filters/InstructionFilter.h"
#include "mull/Filters/MutationFilter.h"

namespace mull {

class NoDebugInfoFilter : public MutationFilter,
                          public FunctionFilter,
                          public InstructionFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(llvm::Function *function) override;
  bool shouldSkip(llvm::Instruction *instruction) const override;
  std::string name() override;
  virtual ~NoDebugInfoFilter() {};
};
} // namespace mull