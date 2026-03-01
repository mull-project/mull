#pragma once

#include "mull/Bitcode.h"

#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
class MemoryBuffer;
} // namespace llvm
struct MullDiagnostics;
namespace mull {

std::unique_ptr<llvm::Module> loadModuleFromBuffer(llvm::LLVMContext &context,
                                                   llvm::MemoryBuffer &buffer,
                                                   const MullDiagnostics &diagnostics);
std::unique_ptr<llvm::Module> parseBitcode(llvm::MemoryBufferRef bufferRef,
                                           llvm::LLVMContext &context,
                                           const MullDiagnostics &diagnostics);

class BitcodeLoader {
  std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;

public:
  BitcodeLoader() = default;
  ~BitcodeLoader() = default;

  std::unique_ptr<Bitcode> loadBitcodeAtPath(const std::string &path,
                                             const MullDiagnostics &diagnostics);
};

} // namespace mull
