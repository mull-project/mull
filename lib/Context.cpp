#include "Context.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"

using namespace Mutang;
using namespace llvm;

void Context::addModule(std::unique_ptr<MutangModule> module) {
  for (auto &function : module->getModule()->getFunctionList()) {
    if (!function.isDeclaration()) {
      FunctionsRegistry.insert(std::make_pair(function.getName(), &function));
    }
  }

  moduleRegistry.insert(std::make_pair(module->getModule()->getModuleIdentifier(),
                                       module.get()));
  Modules.emplace_back(std::move(module));
}

llvm::Function *Context::lookupDefinedFunction(llvm::StringRef FunctionName) {
  auto it = FunctionsRegistry.find(FunctionName.str());
  if (it == FunctionsRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

MutangModule *Context::moduleWithIdentifier(const std::string &identifier) {
  auto it = moduleRegistry.find(identifier);
  if (it == moduleRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

MutangModule *Context::moduleWithIdentifier(const std::string &identifier) const {
  auto it = moduleRegistry.find(identifier);
  if (it == moduleRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

std::vector<llvm::Function *> Context::getStaticConstructors() {
  /// NOTE: Just Copied the whole logic from ExecutionEngine
  std::vector<llvm::Function *> Ctors;

  for (auto &module : Modules) {

    GlobalVariable *GV = module->getModule()->getNamedGlobal("llvm.global_ctors");

    // If this global has internal linkage, or if it has a use, then it must be
    // an old-style (llvmgcc3) static ctor with __main linked in and in use.  If
    // this is the case, don't execute any of the global ctors, __main will do
    // it.
    if (!GV || GV->isDeclaration() || GV->hasLocalLinkage()) continue;

    // Should be an array of '{ i32, void ()* }' structs.  The first value is
    // the init priority, which we ignore.
    ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
    if (!InitList)
      continue;
    for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i) {
      ConstantStruct *CS = dyn_cast<ConstantStruct>(InitList->getOperand(i));
      if (!CS) continue;

      Constant *FP = CS->getOperand(1);
      if (FP->isNullValue())
        continue;  // Found a sentinal value, ignore.

      // Strip off constant expression casts.
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(FP))
        if (CE->isCast())
          FP = CE->getOperand(0);

      // Execute the ctor/dtor function!
      if (Function *F = dyn_cast<Function>(FP))
        Ctors.push_back(F);

      // FIXME: It is marginally lame that we just do nothing here if we see an
      // entry we don't recognize. It might not be unreasonable for the verifier
      // to not even allow this and just assert here.
    }
  }
  
  return Ctors;
}
