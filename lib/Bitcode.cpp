#include "mull/Bitcode.h"

#include "mull/MutationPoint.h"
#include "mull/Parallelization/Parallelization.h"

#include <cassert>
#include <llvm/IR/Module.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace mull;
using namespace llvm;
using namespace std;

Bitcode::Bitcode(llvm::Module *unownedModule)
    : context(nullptr), module(nullptr), unownedModule(unownedModule),
      uniqueIdentifier(llvm::sys::path::stem(this->unownedModule->getModuleIdentifier()).str()) {}

Bitcode::Bitcode(std::unique_ptr<llvm::LLVMContext> context, std::unique_ptr<llvm::Module> module)
    : context(std::move(context)), module(std::move(module)), unownedModule(nullptr),
      uniqueIdentifier(llvm::sys::path::stem(this->module->getModuleIdentifier()).str()) {}

llvm::Module *Bitcode::getModule() {
  assert(module || unownedModule);
  if (unownedModule) {
    return unownedModule;
  }
  return module.get();
}

llvm::Module *Bitcode::getModule() const {
  assert(module || unownedModule);
  if (unownedModule) {
    return unownedModule;
  }
  return module.get();
}

std::string Bitcode::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string Bitcode::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

void Bitcode::addMutation(MutationPoint *point) {
  auto function = point->getOriginalFunction();
  mutationPoints[function].push_back(point);
}

std::map<llvm::Function *, std::vector<MutationPoint *>> &Bitcode::getMutationPointsMap() {
  return mutationPoints;
}
