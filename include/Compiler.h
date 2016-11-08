#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

namespace llvm {

class Module;

};

namespace Mutang {

class MutangModule;

class Compiler {
public:
  Compiler();
  llvm::object::OwningBinary<llvm::object::ObjectFile> compileModule(const MutangModule &module);
  llvm::object::OwningBinary<llvm::object::ObjectFile> compileModule(llvm::Module *module);
};
}
