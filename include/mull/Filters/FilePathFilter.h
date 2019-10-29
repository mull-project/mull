#pragma once

#include "MutationFilter.h"
#include "mull/Filters/FunctionFilter.h"

#include <regex>
#include <vector>

namespace mull {

class FilePathFilter : public MutationFilter, public FunctionFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(llvm::Function *function) override;
  std::string name() override;
  void exclude(const std::string &filter);

private:
  std::vector<std::regex> filters;
};
} // namespace mull