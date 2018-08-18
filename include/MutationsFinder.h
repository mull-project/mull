#pragma once

#include <vector>
#include <map>

#include "Mutators/Mutator.h"
#include "MutationPoint.h"
#include "Testee.h"

namespace llvm {
class Function;
}

namespace mull {
class Config;
class Context;
class Filter;
class Testee;

class MutationsFinder {
public:
  explicit MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators);
  std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                 std::vector<MergedTestee> &testees,
                                                 Filter &filter);
private:
  std::vector<std::unique_ptr<Mutator>> mutators;
  std::vector<std::unique_ptr<MutationPoint>> ownedPoints;
};
}
