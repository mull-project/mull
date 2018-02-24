#pragma once

#include <vector>
#include <map>

#include "MutationOperators/Mutator.h"
#include "MutationPoint.h"

namespace llvm {
  class Function;
}

namespace mull {
  class Context;
  class Filter;
  class Testee;

  class MutationsFinder {
    std::vector<std::unique_ptr<Mutator>> operators;
    std::map<llvm::Function *, std::vector<std::unique_ptr<MutationPoint>>> cachedPoints;
  public:
    MutationsFinder(std::vector<std::unique_ptr<Mutator>> operators);
    std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                   Testee &testee,
                                                   Filter &filter);
  };
}
