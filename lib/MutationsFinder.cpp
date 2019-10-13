#include "mull/MutationsFinder.h"

#include "mull/Config/Configuration.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"
#include "mull/Testee.h"

using namespace mull;
using namespace llvm;

MutationsFinder::MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators,
                                 const Configuration &config,
                                 const FilePathFilter &filePathFilter,
                                 const InstructionFilter &instructionFilter)
    : mutators(std::move(mutators)), config(config),
      filePathFilter(filePathFilter), instructionFilter(instructionFilter) {}

std::vector<MutationPoint *>
MutationsFinder::getMutationPoints(const Program &program,
                                   std::vector<MergedTestee> &testees,
                                   Filter &filter) {
  std::vector<SearchMutationPointsTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(filter, program, mutators, filePathFilter,
                       instructionFilter);
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
