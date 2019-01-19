#pragma once

#include "MullModule.h"
#include "Context.h"

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
  Context &context();

private:
  std::vector<std::string> _dynamicLibraries;
  ObjectFiles _precompiledObjectFiles;
  Context _context;
};

}
