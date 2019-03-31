#include "mull/Parallelization/Progress.h"

#include <llvm/Support/raw_ostream.h>
#include <unistd.h>
#include <vector>

using namespace mull;

progress_counter::progress_counter(const progress_counter &v)
    : value(v.value.load()) {}

void progress_counter::increment() { value++; }

progress_counter::CounterType progress_counter::get() { return value.load(); }

progress_reporter::progress_reporter(std::string &name,
                                     std::vector<progress_counter> &counters,
                                     progress_counter::CounterType total,
                                     size_t workers, llvm::raw_ostream &stream)
    : counters(counters), stream(stream), total(total), previousValue(0),
      name(name), workers(workers) {
  hasTerminal = getenv("TERM") != nullptr;
  bool forceReport = true;
  printProgress(0, total, forceReport);
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
                                      progress_counter::CounterType total,
                                      bool force) {
  if (current == previousValue && !force) {
    return;
  }

  char terminator = '\n';
  if (hasTerminal) {
    terminator = '\r';
  }

  const char *format = "%c%s (threads: %d): %zu/%zu";
  const size_t bufferSize(128);
  char message[bufferSize];
  snprintf(message, bufferSize, format, terminator, name.c_str(), workers,
           current, total);
  stream << message;
  stream.flush();
  previousValue = current;
}
