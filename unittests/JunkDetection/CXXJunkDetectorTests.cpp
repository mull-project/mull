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
#include "JunkDetection/CXX/CXXJunkDetector.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(CXXJunkDetector, boundary_mutator) {
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

  CXXJunkDetector detector;
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: allMutationPoints) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 6U);
}
