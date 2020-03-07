#pragma once

#include "mull/Filters/FunctionFilter.h"
#include "mull/Filters/InstructionFilter.h"
#include "mull/Filters/MutationFilter.h"

#include <mutex>
#include <regex>
#include <unordered_map>
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
  bool shouldSkip(const std::string &sourceFilePath) const;

  std::string name() override;
  void exclude(const std::string &filter);
  void include(const std::string &filter);

private:
  bool shouldSkip(const mull::SourceLocation &location) const;

  std::vector<std::regex> includeFilters;
  std::vector<std::regex> excludeFilters;

  mutable std::unordered_map<std::string, bool> cache;
  mutable std::mutex cacheMutex;
};
} // namespace mull