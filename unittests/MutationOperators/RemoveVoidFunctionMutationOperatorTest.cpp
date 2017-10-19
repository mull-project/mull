
#include "MutationOperators/MutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "Context.h"

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

#include "TestModuleFactory.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static LLVMContext Ctx;

TEST(RemoveVoidFunctionMutationOperator, canBeApplied) {
  /// Does this test make sense at all?
  LLVMContext context;

  std::unique_ptr<Module> moduleOwner(new Module("test", context));
  Module *module = moduleOwner.get();

  Function *voidFunction =
    cast<Function>(module->getOrInsertFunction("voidFunction", Type::getVoidTy(context),
                                        Type::VoidTyID,
                                        nullptr));

  Function *callerFunction =
    cast<Function>(module->getOrInsertFunction("callerFunction", Type::getVoidTy(context),
                                               Type::VoidTyID,
                                               nullptr));

  BasicBlock *bb = BasicBlock::Create(context, "not_relevant", callerFunction);

  RemoveVoidFunctionMutationOperator mutationOperator;

  CallInst *callInst = CallInst::Create(voidFunction, "", bb);

  EXPECT_EQ(true, mutationOperator.canBeApplied(*callInst));
}
