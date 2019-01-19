#pragma once

#include "MullModule.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Object/ObjectFile.h>

#include <vector>

namespace mull {

typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>
    ObjectFiles;

class Program {
public:
  Program(const std::vector<std::string> &dynamicLibraryPaths,
          ObjectFiles precompiledObjectFiles,
          std::vector<std::unique_ptr<MullModule>> modules);

  ObjectFiles &precompiledObjectFiles();
  std::vector<std::unique_ptr<MullModule>> &modules();
  MullModule *moduleWithIdentifier(const std::string &identifier) const;
  llvm::Function *lookupDefinedFunction(llvm::StringRef FunctionName) const;
  std::vector<llvm::Function *> getStaticConstructors() const;

private:
  void addModule(std::unique_ptr<MullModule> module);

  std::vector<std::string> _dynamicLibraries;
  ObjectFiles _precompiledObjectFiles;
  std::vector<std::unique_ptr<MullModule>> _modules;
  std::map<std::string, llvm::Function *> functionsRegistry;
  std::map<std::string, MullModule *> moduleRegistry;
};

} // namespace mull
