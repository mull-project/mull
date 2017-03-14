
#include "Config.h"
#include "Context.h"
#include "ForkProcessSandbox.h"
#include "MutationOperators/AddMutationOperator.h"
#include "Rust/RustTestFinder.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "Rust/RustTestRunner.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(RustTestRunner, runTest) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(
                                EngineBuilder().selectTarget(Triple(), "", "",
                                SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());
  Context context;

  RustTestRunner Runner(*targetMachine.get());
  RustTestRunner::ObjectFiles ObjectFiles;
  RustTestRunner::OwnedObjectFiles OwnedObjectFiles;

  auto ownedModuleWithTests = TestModuleFactory.rustModule();

  Module *moduleWithTests = ownedModuleWithTests.get();

  auto ownedMullModuleWithTests =
    make_unique<MullModule>(std::move(ownedModuleWithTests), "");

  context.addModule(std::move(ownedMullModuleWithTests));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  RustTestFinder testFinder(std::move(mutationOperators), {});

  auto Tests = testFinder.findTests(context);

  ASSERT_EQ(4U, Tests.size());

  auto &Test = *(Tests.begin());

  {
    auto Obj = compiler.compileModule(moduleWithTests);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get(), ObjectFiles).Status);

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());

  std::vector<std::unique_ptr<Testee>> testees =
    testFinder.findTestees(Test.get(), context, 4);

  ASSERT_EQ(2U, testees.size());
  Function *testeeFunction = testees[1]->getTesteeFunction();

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});
  std::vector<MutationPoint *> mutationPoints =
    testFinder.findMutationPoints(context, *testeeFunction);

  ASSERT_EQ(1U, mutationPoints.size());

  auto &mutationPoint = *(mutationPoints.begin());

  {
    LLVMContext mutationContext;
    auto ownedModule = mutationPoint->cloneModuleAndApplyMutation(mutationContext);

    auto owningObject = compiler.compileModule(ownedModule.get());

    auto mutant = owningObject.getBinary();

    ObjectFiles.push_back(mutant);
    OwnedObjectFiles.push_back(std::move(owningObject));
  }

  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    ExecutionResult R = Runner.runTest(Test.get(), ObjectFiles);

    *SharedResult = R;
  }, MullDefaultTimeoutMilliseconds);

  ASSERT_EQ(ExecutionStatus::Crashed, result.Status);

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());
}

TEST(RustTestRunner, runTest_twoModules) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(
    EngineBuilder().selectTarget(Triple(),
                                 "",
                                 "",
                                 SmallVector<std::string, 1>())
  );

  Compiler compiler(*targetMachine.get());
  Context context;

  RustTestRunner Runner(*targetMachine.get());
  RustTestRunner::ObjectFiles ObjectFiles;
  RustTestRunner::OwnedObjectFiles OwnedObjectFiles;

  auto ownedModuleWithTests1 = TestModuleFactory.rustModule();
  auto ownedModuleWithTests2 = TestModuleFactory.rustModule();
  ownedModuleWithTests2->setModuleIdentifier("rust2");

  Module *moduleWithTests1 = ownedModuleWithTests1.get();
  Module *moduleWithTests2 = ownedModuleWithTests2.get();

  auto ownedMullModuleWithTests1 =
    make_unique<MullModule>(std::move(ownedModuleWithTests1), "");
  auto ownedMullModuleWithTests2 =
    make_unique<MullModule>(std::move(ownedModuleWithTests2), "");

  context.addModule(std::move(ownedMullModuleWithTests1));
  context.addModule(std::move(ownedMullModuleWithTests2));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  RustTestFinder testFinder(std::move(mutationOperators), {});

  auto Tests = testFinder.findTests(context);

  ASSERT_EQ(8U, Tests.size());

  auto &Test = *(Tests.begin());

  {
    auto Obj = compiler.compileModule(moduleWithTests1);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }
  {
    auto Obj = compiler.compileModule(moduleWithTests2);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  /// Here we run test with original testee function
  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get(), ObjectFiles).Status);

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());

  std::vector<std::unique_ptr<Testee>> testees =
    testFinder.findTestees(Test.get(), context, 4);

  ASSERT_EQ(2U, testees.size());
  Function *testeeFunction = testees[1]->getTesteeFunction();

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});
  std::vector<MutationPoint *> mutationPoints =
  testFinder.findMutationPoints(context, *testeeFunction);

  ASSERT_EQ(1U, mutationPoints.size());

  auto &mutationPoint = *(mutationPoints.begin());

  {
    LLVMContext mutationContext;
    auto ownedModule = mutationPoint->cloneModuleAndApplyMutation(mutationContext);

    auto owningObject = compiler.compileModule(ownedModule.get());

    auto mutant = owningObject.getBinary();

    ObjectFiles.push_back(mutant);
    OwnedObjectFiles.push_back(std::move(owningObject));
  }

  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    ExecutionResult R = Runner.runTest(Test.get(), ObjectFiles);

    *SharedResult = R;
  }, MullDefaultTimeoutMilliseconds);

  ASSERT_EQ(ExecutionStatus::Crashed, result.Status);
  
  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());
}
