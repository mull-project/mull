#include "Program/Program.h"

#include <llvm/IR/Constants.h>

using namespace mull;

Program::Program(const std::vector<std::string> &dynamicLibraryPaths,
                 ObjectFiles precompiledObjectFiles,
                 std::vector<std::unique_ptr<MullModule>> modules)
    : _dynamicLibraries(dynamicLibraryPaths),
      _precompiledObjectFiles(std::move(precompiledObjectFiles)) {
  for (auto &module : modules) {
    addModule(std::move(module));
  }
}

ObjectFiles &Program::precompiledObjectFiles() {
  return _precompiledObjectFiles;
}

std::vector<std::unique_ptr<MullModule>> &Program::modules() {
  return _modules;
}

void Program::addModule(std::unique_ptr<MullModule> module) {
  for (auto &function : module->getModule()->getFunctionList()) {
    if (function.getName().equals("mull_enterFunction") ||
        function.getName().equals("mull_leaveFunction")) {
      function.deleteBody();
    }

    if (!function.isDeclaration()) {
      functionsRegistry.insert(std::make_pair(function.getName(), &function));
    }
  }

  for (auto &alias : module->getModule()->getAliasList()) {
    if (auto function = llvm::dyn_cast<llvm::Function>(alias.getAliasee())) {
      functionsRegistry.insert(std::make_pair(alias.getName(), function));
    }
  }

  std::string identifier = module->getModule()->getModuleIdentifier();

  assert(moduleWithIdentifier(identifier) == nullptr &&
         "Attempt to add a module which has been added already!");

  moduleRegistry.insert(std::make_pair(identifier, module.get()));
  _modules.emplace_back(std::move(module));
}

MullModule *Program::moduleWithIdentifier(const std::string &identifier) const {
  auto it = moduleRegistry.find(identifier);
  if (it == moduleRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

llvm::Function *
Program::lookupDefinedFunction(llvm::StringRef FunctionName) const {
  auto it = functionsRegistry.find(FunctionName.str());
  if (it == functionsRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

std::vector<llvm::Function *> Program::getStaticConstructors() const {
  using namespace llvm;
  /// NOTE: Just Copied the whole logic from ExecutionEngine
  std::vector<Function *> Ctors;

  for (auto &module : _modules) {

    GlobalVariable *GV =
        module->getModule()->getNamedGlobal("llvm.global_ctors");

    // If this global has internal linkage, or if it has a use, then it must be
    // an old-style (llvmgcc3) static ctor with __main linked in and in use.  If
    // this is the case, don't execute any of the global ctors, __main will do
    // it.
    if (!GV || GV->isDeclaration() || GV->hasLocalLinkage())
      continue;

    // Should be an array of '{ i32, void ()* }' structs.  The first value is
    // the init priority, which we ignore.
    ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
    if (!InitList)
      continue;
    for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i) {
      ConstantStruct *CS = dyn_cast<ConstantStruct>(InitList->getOperand(i));
      if (!CS)
        continue;

      Constant *FP = CS->getOperand(1);
      if (FP->isNullValue())
        continue; // Found a sentinal value, ignore.

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

const std::vector<std::string> &Program::getDynamicLibraryPaths() const {
  return _dynamicLibraries;
}
