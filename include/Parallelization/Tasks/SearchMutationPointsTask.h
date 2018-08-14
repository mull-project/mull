#pragma once

#include "Testee.h"
#include "MutationPoint.h"
#include "Mutators/Mutator.h"

namespace mull {

class Filter;
class Context;
class progress_counter;

class SearchMutationPointsTask {
public:
  using In = const std::vector<MergedTestee>;
  using Out = std::vector<std::unique_ptr<MutationPoint>>;
  using iterator = In::const_iterator;

  SearchMutationPointsTask(Filter &filter, const Context &context, std::vector<std::unique_ptr<Mutator>> &mutators);
  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter);
private:
  Filter &filter;
  const Context &context;
  std::vector<std::unique_ptr<Mutator>> &mutators;
};
}
