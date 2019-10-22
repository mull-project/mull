#include "mull/Parallelization/Tasks/SearchMutationPointsTask.h"

#include "mull/Filter.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Program/Program.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <vector>

using namespace mull;
using namespace llvm;

static int GetFunctionIndex(llvm::Function *function) {
  Module *parent = function->getParent();

  auto functionIterator =
      std::find_if(parent->begin(), parent->end(),
                   [function](llvm::Function &f) { return &f == function; });

  assert(functionIterator != parent->end() &&
         "Expected function to be found in module");
  int index =
      static_cast<int>(std::distance(parent->begin(), functionIterator));

  return index;
}

SearchMutationPointsTask::SearchMutationPointsTask(
    Filter &filter, const Program &program,
    std::vector<std::unique_ptr<Mutator>> &mutators)
    : filter(filter), program(program), mutators(mutators) {}

void SearchMutationPointsTask::
operator()(iterator begin, iterator end,
           std::vector<std::unique_ptr<MutationPoint>> &storage,
           progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &testee = *it;
    Function *function = testee.getTesteeFunction();
    auto moduleID = function->getParent()->getModuleIdentifier();
    Bitcode *bitcode = program.bitcodeWithIdentifier(moduleID);

    for (auto &mutator : mutators) {
      auto mutants = mutator->getMutations(bitcode, function);
      for (auto mutant : mutants) {
        for (auto &reachableTest : testee.getReachableTests()) {
          mutant->addReachableTest(reachableTest.first, reachableTest.second);
        }
        storage.push_back(std::unique_ptr<MutationPoint>(mutant));
      }
    }
  }
}
