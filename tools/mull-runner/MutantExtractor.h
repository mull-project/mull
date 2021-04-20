#pragma once

#include <memory>
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/Mutant.h>
#include <vector>

namespace mull {

class MutantExtractor {
public:
  explicit MutantExtractor(Diagnostics &diagnostics);
  std::vector<std::unique_ptr<Mutant>> extractMutants(const std::string &executable);

private:
  Diagnostics &diagnostics;
};

} // namespace mull
