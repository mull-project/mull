#pragma once

#include "Bitcode.h"

#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
class MemoryBuffer;
} // namespace llvm

namespace mull {

struct Configuration;

std::pair<std::string, std::unique_ptr<llvm::Module>>
loadModuleFromBuffer(llvm::LLVMContext &context, llvm::MemoryBuffer &buffer);

class BitcodeLoader {
  std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;

public:
  BitcodeLoader() = default;
  ~BitcodeLoader() = default;

  std::unique_ptr<Bitcode> loadBitcodeAtPath(const std::string &path, llvm::LLVMContext &context,
                                             Diagnostics &diagnostics);

  std::vector<std::unique_ptr<Bitcode>> loadBitcode(const Configuration &config,
                                                    Diagnostics &diagnostics);
};

} // namespace mull
