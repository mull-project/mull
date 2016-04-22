#include "Context.h"

#include "llvm/IR/Module.h"

using namespace Mutang;
using namespace llvm;

void Context::addModule(Module *M) {
  Modules.push_back(M);
}
