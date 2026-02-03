#include "mull/MutationsFinder.h"

#include "mull/FunctionUnderTest.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"

#include "rust/mull-core/core.rs.h"

using namespace mull;
using namespace llvm;

MutationsFinder::MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                                 const MullConfig &config)
    : mutators(std::move(mutators)), config(config) {}

std::vector<MutationPoint *>
MutationsFinder::getMutationPoints(const MullDiagnostics &diagnostics,
                                   std::vector<FunctionUnderTest> &functions) {
  std::vector<SearchMutationPointsTask> tasks;
  tasks.reserve(config.workers);
  for (unsigned i = 0; i < config.workers; i++) {
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
