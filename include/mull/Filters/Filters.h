#pragma once

#include <vector>

namespace mull {

class FunctionFilter;
class MutationFilter;

struct Filters {
  Filters() : functionFilters(), mutationFilters() {}

  std::vector<FunctionFilter *> functionFilters;
  std::vector<MutationFilter *> mutationFilters;
};

} // namespace mull