#pragma once

#include "mull/Filters/MutantFilter.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct MullConfig;

namespace mull {

class Mutant;
class Diagnostics;

class CoverageFilter : public MutantFilter {
public:
  CoverageFilter(const MullConfig &configuration, Diagnostics &diagnostics,
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
  const MullConfig &configuration;
  std::unordered_map<std::string, std::vector<CoverageRange>> uncoveredRanges;
};

} // namespace mull
