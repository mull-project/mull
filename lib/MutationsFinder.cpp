#include "mull/MutationsFinder.h"

#include "mull/Config/Configuration.h"
#include "mull/FunctionUnderTest.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"

using namespace mull;
using namespace llvm;

MutationsFinder::MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                                 const Configuration &config)
    : mutators(std::move(mutators)), config(config) {}

std::vector<MutationPoint *>
MutationsFinder::getMutationPoints(Diagnostics &diagnostics,
                                   std::vector<FunctionUnderTest> &functions) {
  std::vector<SearchMutationPointsTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(mutators);
  }

  TaskExecutor<SearchMutationPointsTask> finder(
      diagnostics, "Searching mutants across functions", functions, ownedPoints, tasks);
  finder.execute();

  std::vector<MutationPoint *> mutationPoints;
  for (auto &point : ownedPoints) {
    mutationPoints.push_back(point.get());
  }

  return mutationPoints;
}
