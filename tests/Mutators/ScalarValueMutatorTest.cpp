#include "mull/Mutators/ScalarValueMutator.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/MutationPoint.h"
#include "mull/ReachableFunction.h"

#include <gtest/gtest.h>
#include <irm/irm.h>
#include <llvm/IR/LLVMContext.h>
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;
using namespace llvm;

TEST(ScalarValueMutator, getMutationPoint) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_scalar_value_module_bc_path(), context, diagnostics);

  ScalarValueMutator mutator;
  FunctionUnderTest functionUnderTest(
      bitcode->getModule()->getFunction("scalar_value"), nullptr, 0);
  functionUnderTest.selectInstructions({});
  auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);

  ASSERT_EQ(mutants.size(), 4U);

  ASSERT_TRUE(isa<StoreInst>(mutants[0]->getOriginalValue()));
  ASSERT_TRUE(isa<StoreInst>(mutants[1]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutants[2]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutants[3]->getOriginalValue()));

  ASSERT_EQ(mutants[0]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[0]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[0]->getAddress().getIIndex(), 4);

  ASSERT_EQ(mutants[1]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[1]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[1]->getAddress().getIIndex(), 5);

  ASSERT_EQ(mutants[2]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[2]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[2]->getAddress().getIIndex(), 9);

  ASSERT_EQ(mutants[3]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[3]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[3]->getAddress().getIIndex(), 12);
}
