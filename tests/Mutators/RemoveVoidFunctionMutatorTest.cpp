#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "LLVMCompatibility.h"
#include "TestModuleFactory.h"
#include "mull/Mutators/Mutator.h"

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

static LLVMContext Ctx;

TEST(RemoveVoidFunctionMutator, canBeApplied) {
  /// Does this test make sense at all?
  LLVMContext context;

  std::unique_ptr<Module> moduleOwner(new Module("test", context));
  Module *module = moduleOwner.get();

  Type *voidType = Type::getVoidTy(context);
  FunctionType *functionType = FunctionType::get(voidType, voidType);

  Function *voidFunction =
      llvm_compat::GetOrInsertFunction(*module, "voidFunction", functionType);
  Function *callerFunction =
      llvm_compat::GetOrInsertFunction(*module, "callerFunction", functionType);

  BasicBlock *bb = BasicBlock::Create(context, "not_relevant", callerFunction);

  RemoveVoidFunctionMutator mutator;

  CallInst *callInst = CallInst::Create(voidFunction, "", bb);

  EXPECT_EQ(true, mutator.canBeApplied(*callInst));
}
