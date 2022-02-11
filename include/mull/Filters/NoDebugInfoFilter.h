#pragma once

#include "mull/Filters/FunctionFilter.h"
#include "mull/Filters/InstructionFilter.h"
#include "mull/Filters/MutationPointFilter.h"

namespace mull {

class NoDebugInfoFilter : public MutationPointFilter,
                          public FunctionFilter,
                          public InstructionFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(llvm::Function *function) override;
  bool shouldSkip(llvm::Instruction *instruction) override;
  std::string name() override;
  virtual ~NoDebugInfoFilter() {};
};
} // namespace mull
