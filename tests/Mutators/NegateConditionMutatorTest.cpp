#include "mull/Mutators/NegateConditionMutator.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/MutationPoint.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Program/Program.h"
#include "mull/ReachableFunction.h"

#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(NegateConditionMutator, getMutationPoints_no_filter) {
  LLVMContext context;
  auto bitcode = loadBitcodeFromIR(
      fixtures::
          hardcode_APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator_ll_path(),
      context);
  Function *function =
      bitcode->getModule()->getFunction("_ZN4llvm5APInt12tcExtractBitEPKyj");
  assert(function);

  NegateConditionMutator mutator;
  auto mutants = mutator.getMutations(bitcode.get(), function);

  EXPECT_EQ(1U, mutants.size());
  EXPECT_EQ(0, mutants[0]->getAddress().getFnIndex());
  EXPECT_EQ(0, mutants[0]->getAddress().getBBIndex());
  EXPECT_EQ(15, mutants[0]->getAddress().getIIndex());
}
