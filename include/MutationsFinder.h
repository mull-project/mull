#pragma once

#include <vector>
#include <map>

#include "Mutators/Mutator.h"
#include "MutationPoint.h"

namespace llvm {
  class Function;
}

namespace mull {
  class Context;
  class Filter;
  class Testee;

  class MutationsFinder {
    std::vector<std::unique_ptr<Mutator>> mutators;
    std::map<llvm::Function *, std::vector<std::unique_ptr<MutationPoint>>> cachedPoints;
  public:
    explicit MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators);
    std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                   Testee &testee,
                                                   Filter &filter);
  };
}
