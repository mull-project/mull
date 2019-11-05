#pragma once

#include <vector>

namespace mull {

class FunctionFilter;
class InstructionFilter;
class MutationFilter;

struct Filters {
  Filters() : functionFilters(), mutationFilters(), instructionFilters() {}

  std::vector<FunctionFilter *> functionFilters;
  std::vector<MutationFilter *> mutationFilters;
  std::vector<InstructionFilter *> instructionFilters;
};

} // namespace mull