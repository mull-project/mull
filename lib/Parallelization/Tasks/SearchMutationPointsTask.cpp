#include "mull/Parallelization/Tasks/SearchMutationPointsTask.h"

#include "mull/Filter.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Program/Program.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <vector>

using namespace mull;
using namespace llvm;

SearchMutationPointsTask::SearchMutationPointsTask(
    Filter &filter, const Program &program,
    std::vector<std::unique_ptr<Mutator>> &mutators)
    : filter(filter), program(program), mutators(mutators) {}

void SearchMutationPointsTask::operator()(iterator begin, iterator end,
                                          Out &storage,
                                          progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    const FunctionUnderTest &functionUnderTest = *it;
    Function *function = functionUnderTest.getFunction();
    auto moduleID = function->getParent()->getModuleIdentifier();
    Bitcode *bitcode = program.bitcodeWithIdentifier(moduleID);

    for (auto &mutator : mutators) {
      auto mutants = mutator->getMutations(bitcode, functionUnderTest);
      for (auto mutant : mutants) {
        for (auto &reachableTest : functionUnderTest.getReachableTests()) {
          mutant->addReachableTest(reachableTest.first, reachableTest.second);
        }
        storage.push_back(std::unique_ptr<MutationPoint>(mutant));
      }
    }
  }
}
