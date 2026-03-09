#pragma once

#include "Filter.h"
#include <memory>
#include <vector>

struct MullConfig;
struct MullDiagnostics;

namespace mull {

class FunctionFilter;
class InstructionFilter;
class MutationPointFilter;
class MutantFilter;

class Filters {
public:
  Filters(const MullConfig &configuration, const MullDiagnostics &diagnostics);

  /// TODO: hide
  std::vector<FunctionFilter *> functionFilters;
  std::vector<MutationPointFilter *> mutationFilters;
  std::vector<InstructionFilter *> instructionFilters;

  void enableNoDebugFilter();
  void enableFilePathFilter();
  void enableBlockAddressFilter();
  void enableVariadicFunctionFilter();
  void enableManualFilter();

private:
  const MullConfig &configuration;
  const MullDiagnostics &diagnostics;
  std::vector<std::unique_ptr<Filter>> storage;
};

} // namespace mull
