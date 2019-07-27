#include <utility>

#include "mull/Program/Program.h"

#include <llvm/IR/Constants.h>

using namespace mull;

Program::Program(std::vector<std::string> dynamicLibraryPaths,
                 ObjectFiles precompiledObjectFiles,
                 std::vector<std::unique_ptr<Bitcode>> bitcode)
    : _dynamicLibraries(std::move(dynamicLibraryPaths)),
      _precompiledObjectFiles(std::move(precompiledObjectFiles)) {
  for (auto &bc : bitcode) {
    addBitcode(std::move(bc));
  }
}

ObjectFiles &Program::precompiledObjectFiles() {
  return _precompiledObjectFiles;
}

std::vector<std::unique_ptr<Bitcode>> &Program::bitcode() { return _bitcode; }

void Program::addBitcode(std::unique_ptr<Bitcode> bitcode) {
  for (auto &function : bitcode->getModule()->getFunctionList()) {
    if (function.getName().equals("mull_enterFunction") ||
        function.getName().equals("mull_leaveFunction")) {
      function.deleteBody();
    }

    if (!function.isDeclaration()) {
      functionsRegistry.insert(std::make_pair(function.getName(), &function));
    }
  }

  for (auto &alias : bitcode->getModule()->getAliasList()) {
    if (auto function = llvm::dyn_cast<llvm::Function>(alias.getAliasee())) {
      functionsRegistry.insert(std::make_pair(alias.getName(), function));
    }
  }

  std::string identifier = bitcode->getModule()->getModuleIdentifier();

  assert(bitcodeWithIdentifier(identifier) == nullptr &&
         "Attempt to add a module which has been added already!");

  bitcodeRegistry.insert(std::make_pair(identifier, bitcode.get()));
  _bitcode.emplace_back(std::move(bitcode));
}

Bitcode *Program::bitcodeWithIdentifier(const std::string &identifier) const {
  auto it = bitcodeRegistry.find(identifier);
  if (it == bitcodeRegistry.end()) {
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

  for (auto &bitcode : _bitcode) {

    GlobalVariable *GV =
        bitcode->getModule()->getNamedGlobal("llvm.global_ctors");

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
