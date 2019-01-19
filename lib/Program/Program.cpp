
#include <Program/Program.h>

#include "Program/Program.h"

mull::Program::Program(const std::vector<std::string> &dynamicLibraryPaths,
                       mull::ObjectFiles precompiledObjectFiles,
                       std::vector<std::unique_ptr<mull::MullModule>> modules)
    : _dynamicLibraries(dynamicLibraryPaths),
      _precompiledObjectFiles(std::move(precompiledObjectFiles)) {
  for (auto &module : modules) {
    _context.addModule(std::move(module));
  }
}

mull::ObjectFiles &mull::Program::precompiledObjectFiles() {
  return _precompiledObjectFiles;
}

std::vector<std::unique_ptr<mull::MullModule>> &mull::Program::modules() {
  return _context.getModules();
}

mull::Context &mull::Program::context() {
  return _context;
}
