#include "Config.h"
#include "Context.h"
#include "MutationOperators/ConditionalsBoundaryMutator.h"
#include "MutationPoint.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "Toolchain/Toolchain.h"
#include "Filter.h"
#include "Testee.h"
#include "MutationsFinder.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(CXX_BoundaryMutator, findMutations) {
  auto mullModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto module = mullModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> operators;
  operators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(operators));
  Filter filter;

  std::vector<MutationPoint *> allMutationPoints;

  for (auto &function : *module) {
    Testee testee(&function, nullptr, 1);

    std::vector<MutationPoint *> points = finder.getMutationPoints(mullContext, testee, filter);
    std::copy(points.begin(), points.end(), std::back_inserter(allMutationPoints));
  }

  ASSERT_EQ(allMutationPoints.size(), 7U);
}

TEST(CXX_BoundaryMutator, applyMutations) {
  auto mullModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto mutatedModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto borrowedModule = mullModule.get();
  auto module = borrowedModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> operators;
  operators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(operators));
  Filter filter;

  std::vector<MutationPoint *> allMutationPoints;

  for (auto &function : *module) {
    Testee testee(&function, nullptr, 1);

    std::vector<MutationPoint *> points = finder.getMutationPoints(mullContext, testee, filter);
    std::copy(points.begin(), points.end(), std::back_inserter(allMutationPoints));
  }

  for (auto point: allMutationPoints) {
    Instruction *originalInstruction = &point->getAddress().findInstruction(module);
    point->applyMutation(*mutatedModule.get());
    Instruction *mutatedInstruction = &point->getAddress().findInstruction(mutatedModule->getModule());

    if (ConditionalsBoundaryMutator::isGT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGTE(mutatedInstruction));
    }
    else if (ConditionalsBoundaryMutator::isGTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGT(mutatedInstruction));
    }
    else if (ConditionalsBoundaryMutator::isLTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLT(mutatedInstruction));
    }
    else if (ConditionalsBoundaryMutator::isLT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLTE(mutatedInstruction));
    }
  }

  ASSERT_EQ(allMutationPoints.size(), 7U);
}
