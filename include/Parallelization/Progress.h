#pragma once

#include <cstddef>
#include <atomic>
#include <string>
#include <vector>

namespace llvm {
class raw_ostream;
}

namespace mull {
class progress_counter {
public:
  using CounterType = size_t;

  progress_counter() = default;
  progress_counter(const progress_counter &v);
  void increment();
  CounterType get();
private:
  std::atomic<CounterType> value;
};

class progress_reporter {
public:
  progress_reporter(std::string &name,
                    std::vector<progress_counter> &counters,
                    progress_counter::CounterType total,
                    size_t workers,
                    llvm::raw_ostream &stream);

  void operator()();
private:
  void printProgress(progress_counter::CounterType current,
                     progress_counter::CounterType total,
                     bool force);
  void backspace();

  std::vector<progress_counter> &counters;
  llvm::raw_ostream &stream;
  progress_counter::CounterType total;
  progress_counter::CounterType previousValue;
  std::string backspaces;
  bool hasTerminal;
};
}
