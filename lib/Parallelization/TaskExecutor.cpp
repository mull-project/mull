#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Diagnostics/Diagnostics.h"
#include <assert.h>
#include <sstream>

namespace mull {
std::vector<int> taskBatches(size_t itemsCount, size_t tasks) {
  assert(itemsCount >= tasks);

  std::vector<int> result;

  int n = itemsCount;
  int m = tasks;

  int q = 0;
  int r = 0;
  int start = 0;
  int end = 0;
  int k = 0;
  int s = 0;

  for (k = 0; k < m; k++) {
    q = (n - k) / m;
    r = (n - k) % m;

    start = end;
    end = end + q + (r != 0);

    s = s + end - start;
    result.push_back(end - start);
  }

  assert(s == n);
  return result;
}

void printTimeSummary(Diagnostics &diagnostics, MetricsMeasure measure) {
  std::stringstream stringstream;
  stringstream << ". Finished in " << measure.duration() << MetricsMeasure::precision();
  diagnostics.progress(stringstream.str());
}

} // namespace mull