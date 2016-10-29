#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

namespace llvm {

class Module;

};

namespace Mutang {

using namespace llvm;
using namespace llvm::object;

class Compiler {
public:
  Compiler();
  OwningBinary<ObjectFile> compileModule(Module *module);
};
}
