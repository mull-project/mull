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

class Compiler {
public:
  explicit Compiler(Diagnostics &diagnostics);
  llvm::object::OwningBinary<llvm::object::ObjectFile> compileBitcode(const Bitcode &bitcode,
                                                                      llvm::TargetMachine &machine);
  llvm::object::OwningBinary<llvm::object::ObjectFile> compileModule(llvm::Module *module,
                                                                     llvm::TargetMachine &machine);

private:
  Diagnostics &diagnostics;
};
} // namespace mull
