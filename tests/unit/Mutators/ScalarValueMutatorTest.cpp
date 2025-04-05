#include "mull/Mutators/ScalarValueMutator.h"
#include "FixturePaths.h"
#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include "tests/unit/helpers/BitcodeLoader.h"
#include "tests/unit/helpers/TestModuleFactory.h"

#include <gtest/gtest.h>
#include <irm/irm.h>
#include <llvm/IR/LLVMContext.h>
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;
using namespace llvm;

TEST(ScalarValueMutator, getMutationPoint) {
  Diagnostics diagnostics;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::tests_unit_fixtures_mutators_scalar_value_module_c_bc_path(), diagnostics);

  ScalarValueMutator mutator;
  FunctionUnderTest functionUnderTest(bitcode->getModule()->getFunction("scalar_value"),
                                      bitcode.get());
  functionUnderTest.selectInstructions({});
  auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);

  ASSERT_EQ(mutants.size(), 4U);

  ASSERT_TRUE(isa<StoreInst>(mutants[0]->getOriginalValue()));
  ASSERT_TRUE(isa<StoreInst>(mutants[1]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutants[2]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutants[3]->getOriginalValue()));

  ASSERT_EQ(mutants[0]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[0]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[0]->getAddress().getIIndex(), 6);

  ASSERT_EQ(mutants[1]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[1]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[1]->getAddress().getIIndex(), 8);

  ASSERT_EQ(mutants[2]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[2]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[2]->getAddress().getIIndex(), 12);

  ASSERT_EQ(mutants[3]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutants[3]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutants[3]->getAddress().getIIndex(), 15);
}
