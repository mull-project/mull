#pragma once

#include <llvm/ProfileData/Coverage/CoverageMapping.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mull {

struct Configuration;
class Mutant;
class Diagnostics;

class CoverageChecker {
public:
  CoverageChecker(const Configuration &configuration, Diagnostics &diagnostics,
                  const std::string &profileName, const std::vector<std::string> &objects);

  bool covered(Mutant *point);

private:
  struct CoverageRange {
    unsigned lineBegin;
    unsigned columnBegin;
    unsigned lineEnd;
    unsigned columnEnd;
  };
  std::unordered_map<std::string, std::vector<CoverageRange>> uncoveredRanges;
};

} // namespace mull
