#include "mull/Mutators/NegateConditionMutator.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include "mull/Program/Program.h"

#include <cassert>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

TEST(NegateConditionMutator, getMutationPoints_no_filter) {
  auto bitcode = loadBitcodeFromIR(
      fixtures::hardcode_APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator_ll_path());
  Function *function =
      bitcode->getModule()->getFunction("_ZN4llvm5APInt12tcExtractBitEPKyj");
  assert(function);

  NegateConditionMutator mutator;
  FunctionUnderTest functionUnderTest(function, bitcode.get());
  functionUnderTest.selectInstructions({});
  auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);

  EXPECT_EQ(1U, mutants.size());
  EXPECT_EQ(0, mutants[0]->getAddress().getFnIndex());
  EXPECT_EQ(0, mutants[0]->getAddress().getBBIndex());
  EXPECT_EQ(15, mutants[0]->getAddress().getIIndex());
}
