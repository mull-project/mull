#pragma once

#include "mull/Bitcode.h"
#include <string>
#include <vector>

namespace mull {
class Toolchain;
class progress_counter;

class OriginalCompilationTask {
public:
  using In = std::vector<std::unique_ptr<Bitcode>>;
  using Out = std::vector<std::string>;
  using iterator = In::const_iterator;

  explicit OriginalCompilationTask(Toolchain &toolchain);

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);

private:
  Toolchain &toolchain;
};
} // namespace mull
