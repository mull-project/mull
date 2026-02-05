#pragma once

#include <map>
#include <vector>

#include "FunctionUnderTest.h"
#include "MutationPoint.h"
#include "mull/Mutators/Mutator.h"

struct MullConfig;

namespace mull {

class Program;
class Diagnostics;
class Bitcode;

class MutationsFinder {
public:
  MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators, const MullConfig &config);
  std::vector<MutationPoint *> getMutationPoints(Diagnostics &diagnostics,
                                                 std::vector<FunctionUnderTest> &functions);

private:
  std::vector<std::unique_ptr<Mutator>> mutators;
  std::vector<std::unique_ptr<MutationPoint>> ownedPoints;
  const MullConfig &config;
};
} // namespace mull
