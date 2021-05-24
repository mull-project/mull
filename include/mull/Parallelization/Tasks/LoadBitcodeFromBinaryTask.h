#pragma once

#include <llvm/IR/LLVMContext.h>

#include <vector>

namespace ebc {
class EmbeddedFile;
}

namespace mull {

class Bitcode;
class progress_counter;
class Diagnostics;

class LoadBitcodeFromBinaryTask {
public:
  using In = std::vector<std::unique_ptr<ebc::EmbeddedFile>>;
  using Out = std::vector<std::unique_ptr<mull::Bitcode>>;
  using iterator = In::iterator;

  explicit LoadBitcodeFromBinaryTask(Diagnostics &diagnostics) : diagnostics(diagnostics) {}

  void operator()(iterator begin, iterator end, Out &storage, mull::progress_counter &counter);

private:
  Diagnostics &diagnostics;
};
} // namespace mull
