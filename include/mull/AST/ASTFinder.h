#pragma once

#include "mull/Mutators/MutatorKind.h"

namespace mull {

class Configuration;
class Diagnostics;
class Program;
class ASTStorage;
class FilePathFilter;

class ASTFinder {
public:
  ASTFinder(mull::MutatorKindSet mutatorKindSet);

  void findMutations(Diagnostics &diagnostics,
                     const mull::Configuration &config, mull::Program &program,
                     mull::FilePathFilter &pathFilter, mull::ASTStorage &storage);

private:
  mull::MutatorKindSet mutatorKindSet;
};
} // namespace mull
