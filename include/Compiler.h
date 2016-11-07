#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

#include <string>

namespace llvm {

class Module;

};

namespace Mutang {

using namespace llvm;
using namespace llvm::object;

class MutangModule;

class Compiler {
  bool useCache;
public:
  Compiler(bool cache);
  OwningBinary<ObjectFile> compileModule(const MutangModule &module);
  OwningBinary<ObjectFile> compileModule(MutangModule *module,
                                         const std::string &uniqueID);
  OwningBinary<ObjectFile> compileModule(Module *module,
                                         const std::string &uniqueID);
};
}
