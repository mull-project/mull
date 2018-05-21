
#include "Context.h"
#include "Mutators/MathAddMutator.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "Toolchain/Compiler.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "MutationsFinder.h"
#include "Filter.h"
#include "Testee.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "gtest/gtest.h"

#include "FixturesFactory.h"

using namespace mull;
using namespace llvm;

static LLVMContext globalContext;
static ModuleLoader loader(globalContext);
static FixturesFactory factory(loader);

TEST(SimpleTestRunner, runTest) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(
                                EngineBuilder().selectTarget(Triple(), "", "",
                                SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());
  Context Ctx;
  SimpleTestRunner Runner(*targetMachine.get());
  SimpleTestRunner::ObjectFiles ObjectFiles;
  SimpleTestRunner::OwnedObjectFiles OwnedObjectFiles;

  auto OwnedModuleWithTests   = factory.create_simple_test_count_letters_test_count_letters_bc();
  auto OwnedModuleWithTestees = factory.create_simple_test_count_letters_count_letters_bc();

  Module *ModuleWithTests   = OwnedModuleWithTests->getModule();
  Module *ModuleWithTestees = OwnedModuleWithTestees->getModule();

  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder mutationsFinder(std::move(mutators));
  Filter filter;

  SimpleTestFinder testFinder;

  auto Tests = testFinder.findTests(Ctx, filter);

  ASSERT_NE(0U, Tests.size());

  auto &Test = *(Tests.begin());

  {
    auto Obj = compiler.compileModule(ModuleWithTests);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  {
    auto Obj = compiler.compileModule(ModuleWithTestees);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  /// Here we run test with original testee function
  Runner.loadProgram(ObjectFiles);
  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get()));

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  Function *testeeFunction = Ctx.lookupDefinedFunction("count_letters");
  Testee testee(testeeFunction, Test.get(), 1);

  std::vector<MutationPoint *> MutationPoints =
    mutationsFinder.getMutationPoints(Ctx, testee, filter);

  MutationPoint *MP = (*(MutationPoints.begin()));

  LLVMContext localContext;
  auto ownedMutatedTesteeModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedTesteeModule.get());

  {
    auto Obj = compiler.compileModule(ModuleWithTests);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  {
    auto Obj = compiler.compileModule(ownedMutatedTesteeModule->getModule());
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  Runner.loadProgram(ObjectFiles);
  ASSERT_EQ(ExecutionStatus::Failed, Runner.runTest(Test.get()));

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());
}
