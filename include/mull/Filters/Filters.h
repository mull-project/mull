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

class CoverageFilter;

class Filters {
public:
  Filters(const MullConfig &configuration, const MullDiagnostics &diagnostics);

  /// TODO: hide
  std::vector<FunctionFilter *> functionFilters;
  std::vector<MutationPointFilter *> mutationFilters;
  std::vector<MutantFilter *> mutantFilters;
  std::vector<InstructionFilter *> instructionFilters;

  void enableNoDebugFilter();
  void enableFilePathFilter();
  void enableGitDiffFilter();
  void enableBlockAddressFilter();
  void enableVariadicFunctionFilter();
  void enableManualFilter();
  CoverageFilter *enableCoverageFilter(const std::string &profileName,
                                       const std::vector<std::string> &objects);

private:
  const MullConfig &configuration;
  const MullDiagnostics &diagnostics;
  std::vector<std::unique_ptr<Filter>> storage;
};

} // namespace mull
