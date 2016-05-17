#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

namespace llvm {

class Module;

};

namespace Mutang {
class Compiler {
public:
  Compiler();
  llvm::object::OwningBinary<llvm::object::ObjectFile> CompilerModule(llvm::Module *M);
};
}
