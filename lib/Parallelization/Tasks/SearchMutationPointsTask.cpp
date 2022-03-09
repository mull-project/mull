#include "mull/Parallelization/Tasks/SearchMutationPointsTask.h"

#include "mull/Mutation/Program.h"
#include "mull/Parallelization/Progress.h"

#include <vector>

using namespace mull;
using namespace llvm;

SearchMutationPointsTask::SearchMutationPointsTask(std::vector<std::unique_ptr<Mutator>> &mutators)
    : mutators(mutators) {}

void SearchMutationPointsTask::operator()(iterator begin, iterator end, Out &storage,
                                          progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    FunctionUnderTest &functionUnderTest = *it;
    Bitcode *bitcode = functionUnderTest.getBitcode();

    for (auto &mutator : mutators) {
      auto mutants = mutator->getMutations(bitcode, functionUnderTest);
      for (auto mutant : mutants) {
        storage.push_back(std::unique_ptr<MutationPoint>(mutant));
      }
    }
  }
}
