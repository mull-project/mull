#pragma once

#include <map>
#include <vector>

#include "MutationPoint.h"
#include "ReachableFunction.h"
#include "mull/Mutators/Mutator.h"

namespace mull {
struct Configuration;
class Program;
class Filter;
class ReachableFunction;

class MutationsFinder {
public:
  explicit MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                           const Configuration &config);
  std::vector<MutationPoint *>
  getMutationPoints(const Program &program,
                    std::vector<FunctionUnderTest> &functions, Filter &filter);

private:
  std::vector<std::unique_ptr<Mutator>> mutators;
  std::vector<std::unique_ptr<MutationPoint>> ownedPoints;
  const Configuration &config;
};
} // namespace mull
