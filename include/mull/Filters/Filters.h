#pragma once

#include "Filter.h"
#include <memory>
#include <vector>

namespace mull {

class FunctionFilter;
class InstructionFilter;
class MutationPointFilter;
class MutantFilter;
class Diagnostics;
struct Configuration;

class CoverageFilter;

class Filters {
public:
  Filters(const Configuration &configuration, Diagnostics &diagnostics);

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
  CoverageFilter *enableCoverageFilter(const std::string &profileName,
                                       const std::vector<std::string> &objects);

private:
  const Configuration &configuration;
  Diagnostics &diagnostics;
  std::vector<std::unique_ptr<Filter>> storage;
};

} // namespace mull
