#pragma once

#include "Filter.h"
#include <memory>
#include <vector>

struct MullConfig;
struct MullDiagnostics;

namespace mull {

class InstructionFilter;
class MutationPointFilter;
class MutantFilter;

class Filters {
public:
  Filters(const MullConfig &configuration, const MullDiagnostics &diagnostics);

  /// TODO: hide
  std::vector<MutationPointFilter *> mutationFilters;

  void enableFilePathFilter();

private:
  const MullConfig &configuration;
  const MullDiagnostics &diagnostics;
  std::vector<std::unique_ptr<Filter>> storage;
};

} // namespace mull
