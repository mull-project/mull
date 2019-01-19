#include "MutationsFinder.h"
#include "Config/Configuration.h"
#include "Parallelization/Parallelization.h"
#include "Testee.h"
#include <Program/Program.h>

using namespace mull;
using namespace llvm;

MutationsFinder::MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                                 const Configuration &config)
    : mutators(std::move(mutators)), config(config) {}

std::vector<MutationPoint *>
MutationsFinder::getMutationPoints(const Program &program,
                                   std::vector<MergedTestee> &testees,
                                   Filter &filter) {
  std::vector<SearchMutationPointsTask> tasks;
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(filter, program, mutators);
  }

  TaskExecutor<SearchMutationPointsTask> finder(
      "Searching mutants across functions", testees, ownedPoints, tasks);
  finder.execute();

  std::vector<MutationPoint *> mutationPoints;
  for (auto &point : ownedPoints) {
    mutationPoints.push_back(point.get());
  }

  return mutationPoints;
}
