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
  ASTFinder(const MutatorKindSet &mutatorKindSet);

  void findMutations(Diagnostics &diagnostics, const Configuration &config, Program &program,
                     FilePathFilter &pathFilter, ASTStorage &storage);

private:
  const mull::MutatorKindSet &mutatorKindSet;
};
} // namespace mull
