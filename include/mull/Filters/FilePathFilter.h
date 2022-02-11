#pragma once

#include "mull/Filters/FunctionFilter.h"
#include "mull/Filters/InstructionFilter.h"
#include "mull/Filters/MutantFilter.h"
#include "mull/Filters/MutationPointFilter.h"

#include <llvm/Support/Regex.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mull {
struct SourceLocation;

class FilePathFilter : public MutationPointFilter,
                       public FunctionFilter,
                       public InstructionFilter,
                       public MutantFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(Mutant *point) override;
  bool shouldSkip(llvm::Function *function) override;
  bool shouldSkip(llvm::Instruction *instruction) override;
  bool shouldSkip(const std::string &sourceFilePath) const;

  std::string name() override;
  std::pair<bool, std::string> exclude(const std::string &filter);
  std::pair<bool, std::string> include(const std::string &filter);

private:
  bool shouldSkip(const mull::SourceLocation &location) const;

// The API of llvm::Regex support const match starting from 10.0.0
#if LLVM_VERSION_MAJOR >= 10
  std::vector<llvm::Regex> includeFilters;
  std::vector<llvm::Regex> excludeFilters;
#else
  mutable std::vector<llvm::Regex> includeFilters;
  mutable std::vector<llvm::Regex> excludeFilters;
#endif

  mutable std::unordered_map<std::string, bool> cache;
  mutable std::mutex cacheMutex;
};
} // namespace mull
