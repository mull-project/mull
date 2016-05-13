#include "Context.h"

#include "llvm/IR/Module.h"

using namespace Mutang;
using namespace llvm;

void Context::addModule(std::unique_ptr<Module> M) {
  Modules.emplace_back(std::move(M));
}
