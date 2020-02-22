#include "mull/Parallelization/Progress.h"

#include "mull/Diagnostics/Diagnostics.h"
#include <llvm/Support/raw_ostream.h>
#include <sstream>
#include <unistd.h>
#include <vector>

using namespace mull;

progress_counter::progress_counter(const progress_counter &v) : value(v.value.load()) {}

void progress_counter::increment() {
  value++;
}

progress_counter::CounterType progress_counter::get() {
  return value.load();
}

progress_reporter::progress_reporter(Diagnostics &diagnostics, std::string &name,
                                     std::vector<progress_counter> &counters,
                                     progress_counter::CounterType total, size_t workers)
    : diagnostics(diagnostics), counters(counters), total(total), previousValue(0), name(name),
      workers(workers) {
  hasTerminal = getenv("TERM") != nullptr;
  std::stringstream stringstream;
  stringstream << name << " (threads: " << this->workers << ")";
  diagnostics.info(stringstream.str());
}

void progress_reporter::operator()() {
  useconds_t microseconds(10000);
  for (;; usleep(microseconds)) {
    progress_counter::CounterType current(0);
    for (auto &counter : counters) {
      current += counter.get();
    }

    if (current == 0) {
      continue;
    }

    bool forceReport = false;
    printProgress(current, total, forceReport);

    if (current == total) {
      break;
    }
  }
}

void progress_reporter::printProgress(progress_counter::CounterType current,
                                      progress_counter::CounterType total, bool force) {
  if (current == previousValue) {
    return;
  }

  char terminator = '\n';
  if (hasTerminal) {
    terminator = '\r';
  }

  const size_t barWidth = 32;
  size_t starsCount = (double)current / total * barWidth;
  std::string stars(starsCount, '#');
  std::string dots(barWidth - starsCount, '-');

  std::stringstream stringstream;
  std::string padding(7, ' ');
  stringstream << terminator << padding << "[" << stars << dots << "] " << current << "/" << total;
  diagnostics.progress(stringstream.str());
  previousValue = current;
}
