#pragma once

#include <string>
#include <vector>

#include <llvm/Object/ObjectFile.h>

namespace mull {

class progress_counter;
class Diagnostics;

class LoadObjectFilesTask {
public:
  using In = const std::vector<std::string>;
  using Out = std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>;
  using iterator = In::const_iterator;

  explicit LoadObjectFilesTask(Diagnostics &diagnostics);
  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Diagnostics &diagnostics;
};
} // namespace mull
