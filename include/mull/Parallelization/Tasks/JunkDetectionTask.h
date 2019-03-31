#pragma once

#include <vector>

namespace mull {
class MutationPoint;
class progress_counter;
class JunkDetector;

class JunkDetectionTask {
public:
  using In = std::vector<MutationPoint *>;
  using Out = std::vector<MutationPoint *>;
  using iterator = In::const_iterator;

  JunkDetectionTask(JunkDetector &detector);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  JunkDetector &detector;
};
} // namespace mull
