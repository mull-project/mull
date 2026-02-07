#pragma once

#include <memory>
#include <mull/Mutant.h>
#include <vector>
struct MullDiagnostics;
namespace mull {

class MutantExtractor {
public:
  explicit MutantExtractor(const MullDiagnostics &diagnostics);
  std::vector<std::unique_ptr<Mutant>>
  extractMutants(const std::vector<std::string> &mutantHolders);

private:
  std::vector<std::string> extractMutants(const std::string &executable);
  const MullDiagnostics &diagnostics;
};

} // namespace mull
