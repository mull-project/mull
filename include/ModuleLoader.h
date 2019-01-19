#pragma once

#include "MullModule.h"

#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

namespace mull {

struct Configuration;

class ModuleLoader {
  std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;

public:
  ModuleLoader() = default;
  ~ModuleLoader() = default;

  std::unique_ptr<MullModule> loadModuleAtPath(const std::string &path,
                                               llvm::LLVMContext &context);

  std::vector<std::unique_ptr<MullModule>>
  loadModules(const Configuration &config);
};

} // namespace mull
