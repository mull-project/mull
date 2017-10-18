#pragma once

#include <vector>

#include "MutationOperators/MutationOperator.h"
#include "MutationPoint.h"

namespace mull {
  class Context;
  class Filter;
  class Testee;

  class MutationsFinder {
    std::vector<std::unique_ptr<MutationOperator>> operators;
    std::vector<std::unique_ptr<MutationPoint>> ownedPoints;
  public:
    MutationsFinder(std::vector<std::unique_ptr<MutationOperator>> operators);
    std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                   Testee &testee,
                                                   Filter &filter);
  };
}
