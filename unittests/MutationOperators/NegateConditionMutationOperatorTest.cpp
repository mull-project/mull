
#include "MutationOperators/MutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"

#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static LLVMContext Ctx;

TEST(NegateConditionMutationOperator, canBeApplied) {
  LLVMContext context;

  // Get pointers to the constants.
  Value *One = ConstantInt::get(Type::getInt32Ty(context), 1);
  Value *Two = ConstantInt::get(Type::getInt32Ty(context), 2);

  NegateConditionMutationOperator mutationOperator;

  // Create the "if (arg <= 2) goto exitbb"
  std::unique_ptr<Value> CondInst = make_unique<ICmpInst>(ICmpInst::ICMP_SLE, One, Two, "cond");

  EXPECT_EQ(true, mutationOperator.canBeApplied(*CondInst));
}

TEST(NegateConditionMutationOperator, negatedCmpInstPredicate) {
  //llvm::CmpInst::Predicate::

  EXPECT_EQ(NegateConditionMutationOperator::negatedCmpInstPredicate(CmpInst::ICMP_SLT), CmpInst::ICMP_SGE);
}
