#pragma once

#include "mull/Filters/MutationPointFilter.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Reporters/SourceManager.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct MullDiagnostics;

namespace mull {

class MutationPoint;

struct IgnoreRange {
  size_t begin;
  size_t end;
  std::unordered_set<std::string> ignoredMutators;
};

class ManualFilter : public MutationPointFilter {
public:
  explicit ManualFilter(const MullDiagnostics &diagnostics);
  bool shouldSkip(MutationPoint *mutationPoint) override;
  std::string name() override;
  ~ManualFilter() override = default;

private:
  void scanFile(const mull::SourceLocation &location);
  std::unordered_set<std::string> expandIgnoreLine(const std::string &line,
                                                   const std::string &needle);

  const MullDiagnostics &diagnostics;
  SourceManager sourceManager;
  MutatorsFactory factory;
  std::unordered_map<std::string, std::vector<IgnoreRange>> ignoreRanges;
  std::mutex lock;
};

} // namespace mull
