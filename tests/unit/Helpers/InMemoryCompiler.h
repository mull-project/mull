#pragma once

#include <memory>

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

namespace mull_test {
class InMemoryCompiler {
public:
  std::unique_ptr<llvm::Module>
  compile(const std::string &code, const std::string &inMemoryFileName, llvm::LLVMContext &context);
};
} // namespace mull_test
