#pragma once

#include <map>
#include <vector>

#include "MutationPoint.h"
#include "Mutators/Mutator.h"
#include "Testee.h"

namespace llvm {
class Function;
}

namespace mull {
struct Configuration;
class Program;
class Filter;
class Testee;

class MutationsFinder {
public:
  explicit MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                           const Configuration &config);
  std::vector<MutationPoint *>
  getMutationPoints(const Program &program, std::vector<MergedTestee> &testees,
                    Filter &filter);

private:
  std::vector<std::unique_ptr<Mutator>> mutators;
  std::vector<std::unique_ptr<MutationPoint>> ownedPoints;
  const Configuration &config;
};
} // namespace mull
