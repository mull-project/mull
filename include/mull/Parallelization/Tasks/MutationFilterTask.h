#pragma once

#include <vector>

namespace mull {

class MutationPoint;
class MutationFilter;
class progress_counter;

class MutationFilterTask {
public:
  using In = std::vector<MutationPoint *>;
  using Out = std::vector<MutationPoint *>;
  using iterator = In::const_iterator;

  MutationFilterTask(MutationFilter &filter);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  MutationFilter &filter;
};

} // namespace mull