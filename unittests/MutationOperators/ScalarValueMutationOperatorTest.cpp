
#include "Context.h"
#include "MutationOperators/ScalarValueMutationOperator.h"
#include "MutationPoint.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(ScalarValueMutationOperator, getMutationPoints) {
  auto module = TestModuleFactory.create_SimpleTest_ScalarValue_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context mullContext;
  mullContext.addModule(std::move(mullModule));

  NullMutationOperatorFilter NOT_RELEVANT;
  auto scalarValueFunction = mullContext.lookupDefinedFunction("scalar_value");

  ScalarValueMutationOperator mutationOperator;

  std::vector<MutationPoint *> mutationPoints =
    mutationOperator.getMutationPoints(mullContext,
                                       scalarValueFunction,
                                       NOT_RELEVANT);

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
