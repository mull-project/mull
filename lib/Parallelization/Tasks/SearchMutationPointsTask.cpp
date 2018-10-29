#include "Parallelization/Progress.h"
#include "Parallelization/Tasks/SearchMutationPointsTask.h"
#include "Filter.h"
#include "Context.h"

#include <vector>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

using namespace mull;
using namespace llvm;

static int GetFunctionIndex(llvm::Function *function) {
  Module *parent = function->getParent();

  auto functionIterator = std::find_if(parent->begin(), parent->end(),
                                       [function] (llvm::Function &f) {
                                         return &f == function;
                                       });

  assert(functionIterator != parent->end()
             && "Expected function to be found in module");
  int index = static_cast<int>(std::distance(parent->begin(),
                                             functionIterator));

  return index;
}

SearchMutationPointsTask::SearchMutationPointsTask(Filter &filter, const Context &context, std::vector<std::unique_ptr<Mutator>> &mutators)
    : filter(filter), context(context), mutators(mutators) {

}

void SearchMutationPointsTask::operator()(iterator begin,
                                          iterator end,
                                          std::vector<std::unique_ptr<MutationPoint>> &storage,
                                          progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &testee = *it;
    Function *function = testee.getTesteeFunction();

    auto moduleID = function->getParent()->getModuleIdentifier();
    MullModule *module = context.moduleWithIdentifier(moduleID);

    int functionIndex = GetFunctionIndex(function);
    for (auto &mutator : mutators) {

      int basicBlockIndex = 0;
      for (auto &basicBlock : function->getBasicBlockList()) {

        int instructionIndex = 0;
        for (auto &instruction : basicBlock.getInstList()) {
          if (filter.shouldSkipInstruction(&instruction)) {
            instructionIndex++;
            continue;
          }

          auto location = SourceLocation::sourceLocationFromInstruction(&instruction);

          MutationPointAddress address(functionIndex, basicBlockIndex, instructionIndex);
          MutationPoint *point = mutator->getMutationPoint(module, function, &instruction, location, address);
          if (point) {
            module->addMutation(point);
            for (auto &reachableTest : testee.getReachableTests()) {
              point->addReachableTest(reachableTest.first, reachableTest.second);
            }
            storage.emplace_back(std::unique_ptr<MutationPoint>(point));
          }
          instructionIndex++;
        }
        basicBlockIndex++;
      }
    }
  }
}

