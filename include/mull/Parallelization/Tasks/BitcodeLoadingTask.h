#pragma once

#include "mull/Bitcode.h"

namespace llvm {
class LLVMContext;
}

namespace mull {

class BitcodeLoader;
class progress_counter;

class BitcodeLoadingTask {
public:
  using In = const std::vector<std::string>;
  using Out = std::vector<std::unique_ptr<Bitcode>>;
  using iterator = In::const_iterator;

  BitcodeLoadingTask(Diagnostics &diagnostics, llvm::LLVMContext &context, BitcodeLoader &loader);
  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Diagnostics &diagnostics;
  llvm::LLVMContext &context;
  BitcodeLoader &loader;
};

} // namespace mull
