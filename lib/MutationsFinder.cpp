#include "MutationsFinder.h"
#include "Context.h"
#include "Filter.h"
#include "Testee.h"
#include "MutationPoint.h"
#include "SourceLocation.h"

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

MutationsFinder::MutationsFinder(std::vector<std::unique_ptr<Mutator>> mutators)
: mutators(std::move(mutators)) {}

std::vector<MutationPoint *> MutationsFinder::getMutationPoints(const Context &context,
                                                                std::vector<MergedTestee> &testees,
                                                                Filter &filter) {
  std::vector<MutationPoint *> points;
  for (auto &testee : testees) {
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
          MutationPoint *point = mutator->getMutationPoint(module, address, &instruction, location);
          if (point) {
            for (auto &reachableTest : testee.getReachableTests()) {
              point->addReachableTest(reachableTest.first, reachableTest.second);
            }
            points.push_back(point);
            ownedPoints.emplace_back(std::unique_ptr<MutationPoint>(point));
          }
          instructionIndex++;
        }
        basicBlockIndex++;
      }
    }
  }

  return points;
}
