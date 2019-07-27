#pragma once

#include "mull/Bitcode.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Object/ObjectFile.h>

#include <vector>

namespace mull {

typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>
    ObjectFiles;

class Program {
public:
  Program(std::vector<std::string> dynamicLibraryPaths,
          ObjectFiles precompiledObjectFiles,
          std::vector<std::unique_ptr<Bitcode>> bitcode);

  ObjectFiles &precompiledObjectFiles();
  std::vector<std::unique_ptr<Bitcode>> &bitcode();
  Bitcode *bitcodeWithIdentifier(const std::string &identifier) const;
  llvm::Function *lookupDefinedFunction(llvm::StringRef FunctionName) const;
  std::vector<llvm::Function *> getStaticConstructors() const;

  const std::vector<std::string> &getDynamicLibraryPaths() const;

private:
  void addBitcode(std::unique_ptr<Bitcode> bitcode);

  std::vector<std::string> _dynamicLibraries;
  ObjectFiles _precompiledObjectFiles;
  std::vector<std::unique_ptr<Bitcode>> _bitcode;
  std::map<std::string, llvm::Function *> functionsRegistry;
  std::map<std::string, Bitcode *> bitcodeRegistry;
};

} // namespace mull
