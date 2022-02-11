#pragma once

#include "mull/Filters/MutantFilter.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mull {

struct Configuration;
class Mutant;
class Diagnostics;

class CoverageFilter : public MutantFilter {
public:
  CoverageFilter(const Configuration &configuration, Diagnostics &diagnostics,
                 const std::string &profileName, const std::vector<std::string> &objects);

  bool shouldSkip(Mutant *point) override;
  std::string name() override;

  bool covered(Mutant *point);

private:
  struct CoverageRange {
    unsigned lineBegin;
    unsigned columnBegin;
    unsigned lineEnd;
    unsigned columnEnd;
  };
  const Configuration &configuration;
  std::unordered_map<std::string, std::vector<CoverageRange>> uncoveredRanges;
};

} // namespace mull
