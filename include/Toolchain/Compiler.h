#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

namespace llvm {

class Module;
class TargetMachine;

};

namespace Mutang {

class MutangModule;

class Compiler {
  llvm::TargetMachine &targetMachine;
public:
  Compiler(llvm::TargetMachine &machine);
  llvm::object::OwningBinary<llvm::object::ObjectFile> compileModule(const MutangModule &module);
  llvm::object::OwningBinary<llvm::object::ObjectFile> compileModule(llvm::Module *module);
};
}
