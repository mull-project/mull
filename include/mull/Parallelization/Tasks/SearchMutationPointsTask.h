#pragma once

#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/Mutator.h"

namespace mull {

class progress_counter;

class SearchMutationPointsTask {
public:
  using In = std::vector<FunctionUnderTest>;
  using Out = std::vector<std::unique_ptr<MutationPoint>>;
  using iterator = In::iterator;

  explicit SearchMutationPointsTask(std::vector<std::unique_ptr<Mutator>> &mutators);
  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);

private:
  std::vector<std::unique_ptr<Mutator>> &mutators;
};

} // namespace mull
