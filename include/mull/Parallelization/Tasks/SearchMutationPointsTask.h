#pragma once

#include "mull/MutationPoint.h"
#include "mull/Mutators/Mutator.h"
#include "mull/ReachableFunction.h"

namespace mull {

class Filter;
class Program;
class progress_counter;

class SearchMutationPointsTask {
public:
  using In = const std::vector<FunctionUnderTest>;
  using Out = std::vector<std::unique_ptr<MutationPoint>>;
  using iterator = In::const_iterator;

  SearchMutationPointsTask(Filter &filter, const Program &program,
                           std::vector<std::unique_ptr<Mutator>> &mutators);
  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Filter &filter;
  const Program &program;
  std::vector<std::unique_ptr<Mutator>> &mutators;
};

} // namespace mull
