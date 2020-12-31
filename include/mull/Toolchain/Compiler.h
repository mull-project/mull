#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

namespace llvm {

class Module;
class TargetMachine;

} // namespace llvm

namespace mull {

class Bitcode;
class Diagnostics;
struct Configuration;

class Compiler {
public:
  explicit Compiler(Diagnostics &diagnostics, const Configuration &configuration);
  std::string compileBitcode(const Bitcode &bitcode);

private:
  Diagnostics &diagnostics;
  const Configuration &configuration;
};
} // namespace mull
