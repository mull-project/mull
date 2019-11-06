#pragma once

#include "mull/Filters/FunctionFilter.h"
#include "mull/Filters/InstructionFilter.h"
#include "mull/Filters/MutationFilter.h"
#include <regex>
#include <vector>

namespace mull {
class SourceLocation;

class FilePathFilter : public MutationFilter,
                       public FunctionFilter,
                       public InstructionFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(llvm::Function *function) override;
  bool shouldSkip(llvm::Instruction *instruction) const override;
  std::string name() override;
  void exclude(const std::string &filter);

private:
  bool shouldSkip(const mull::SourceLocation &location) const;

  std::vector<std::regex> filters;
};
} // namespace mull