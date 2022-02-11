#pragma once

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointFilter;
class progress_counter;

class MutationFilterTask {
public:
  using In = std::vector<MutationPoint *>;
  using Out = std::vector<MutationPoint *>;
  using iterator = In::const_iterator;

  MutationFilterTask(MutationPointFilter &filter);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  MutationPointFilter &filter;
};

} // namespace mull
