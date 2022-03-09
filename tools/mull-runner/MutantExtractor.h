#pragma once

#include <memory>
#include <mull/Core/Mutant.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <vector>

namespace mull {

class MutantExtractor {
public:
  explicit MutantExtractor(Diagnostics &diagnostics);
  std::vector<std::unique_ptr<Mutant>>
  extractMutants(const std::vector<std::string> &mutantHolders);

private:
  std::vector<std::string> extractMutants(const std::string &executable);
  Diagnostics &diagnostics;
};

} // namespace mull
