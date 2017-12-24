#pragma once

#include <vector>
#include <map>

#include "MutationOperators/MutationOperator.h"
#include "MutationPoint.h"

namespace llvm {
  class Function;
}

namespace mull {
  class Context;
  class Filter;
  class Testee;

  class MutationsFinder {
    std::vector<std::unique_ptr<MutationOperator>> operators;
    std::map<llvm::Function *, std::vector<std::unique_ptr<MutationPoint>>> cachedPoints;
  public:
    MutationsFinder(std::vector<std::unique_ptr<MutationOperator>> operators);
    std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                   Testee &testee,
                                                   Filter &filter);
  };
}
