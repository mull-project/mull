#include "Parallelization/Progress.h"

#include <vector>
#include <unistd.h>
#include <llvm/Support/raw_ostream.h>

using namespace mull;

progress_counter::progress_counter(const progress_counter &v) : value(v.value.load()) {}

void progress_counter::increment() { value++; }

progress_counter::CounterType progress_counter::get() { return value.load(); }

progress_reporter::progress_reporter(std::string &name,
                                     std::vector<progress_counter> &counters,
                                     progress_counter::CounterType total,
                                     size_t workers,
                                     llvm::raw_ostream &stream)
    : counters(counters),
      stream(stream),
      total(total),
      previousValue(0),
      backspaces(std::to_string(total).size() * 2 + 1,'\b')
{
  std::string message = name + " (threads: " + std::to_string(workers) + "): ";
  stream << message;
  stream.flush();
  hasTerminal = getenv("TERM") != nullptr;
  bool forceReport = true;
  printProgress(0, total, forceReport);
}

void progress_reporter::operator()() {
  for (;;usleep(1000)) {
    progress_counter::CounterType current(0);
    for (auto &counter : counters) {
      current += counter.get();
    }

    if (current == 0) {
      continue;
    }

    backspace();
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

  if (hasTerminal) {
    char buf[128];
    char buf2[128];
    auto width = (backspaces.size() - 1) / 2;
    std::string backspaces(width * 2 + 1, '\b');
    sprintf(buf, "%%%zud/%%%zud", width, width);
    sprintf(buf2, buf, current, total);
    stream << buf2;
  } else {
    auto diff = current - previousValue;
    std::string dots(diff, '.');
    stream << dots;
  }
  previousValue = current;
}

void progress_reporter::backspace() {
  if (!hasTerminal) {
    return;
  }
  stream << backspaces;
  stream.flush();
}
