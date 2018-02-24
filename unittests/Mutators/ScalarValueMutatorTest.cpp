#include "Config.h"
#include "Context.h"
#include "Mutators/ScalarValueMutator.h"
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

TEST(ScalarValueMutator, getMutationPoint) {
  auto mullModule = TestModuleFactory.create_SimpleTest_ScalarValue_Module();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));

  auto scalarValueFunction = mullContext.lookupDefinedFunction("scalar_value");
  Testee testee(scalarValueFunction, nullptr, 1);
  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ScalarValueMutator>());
  MutationsFinder finder(std::move(mutators));
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(mullContext,
                                                                         testee,
                                                                         filter);

  ASSERT_EQ(mutationPoints.size(), 4U);

  ASSERT_TRUE(isa<StoreInst>(mutationPoints[0]->getOriginalValue()));
  ASSERT_TRUE(isa<StoreInst>(mutationPoints[1]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoints[2]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoints[3]->getOriginalValue()));

  ASSERT_EQ(mutationPoints[0]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[0]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[0]->getAddress().getIIndex(), 4);

  ASSERT_EQ(mutationPoints[1]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[1]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[1]->getAddress().getIIndex(), 5);

  ASSERT_EQ(mutationPoints[2]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[2]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[2]->getAddress().getIIndex(), 9);

  ASSERT_EQ(mutationPoints[3]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[3]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[3]->getAddress().getIIndex(), 12);
}

TEST(DISABLED_ScalarValueMutator, failingMutationPoint) {
  auto mullModule = TestModuleFactory.create_CustomTest_OpenSSL_bio_enc_test_Module();

  MutationPointAddress address(15, 10, 7);
  ScalarValueMutator mutator;
  MutationPoint point(&mutator, address, nullptr, mullModule.get());

  Config config;
  Toolchain toolchain(config);
//  auto mutant = point.cloneModuleAndApplyMutation();
//  toolchain.compiler().compileModule(mutant.get());
}
