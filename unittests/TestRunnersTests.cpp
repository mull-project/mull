#include "MutationEngine.h"

#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "SimpleTest/SimpleTestRunner.h"

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

using namespace Mutang;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

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

  auto OwnedModuleWithTests   = TestModuleFactory.createTesterModule();
  auto OwnedModuleWithTestees = TestModuleFactory.createTesteeModule();

  Module *ModuleWithTests   = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  auto mutangOwnedModuleWithTests   = make_unique<MutangModule>(std::move(OwnedModuleWithTests), "");
  auto mutangOwnedModuleWithTestees = make_unique<MutangModule>(std::move(OwnedModuleWithTestees), "");

  Ctx.addModule(std::move(mutangOwnedModuleWithTests));
  Ctx.addModule(std::move(mutangOwnedModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  auto Tests = testFinder.findTests(Ctx);

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
  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get(), ObjectFiles).Status);

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  std::vector<std::unique_ptr<Testee>> Testees = testFinder.findTestees(Test.get(), Ctx, 4);

  ASSERT_NE(0U, Testees.size());
  Function *Testee = Testees[1]->getTesteeFunction();

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<MutationPoint *> MutationPoints = testFinder.findMutationPoints(Ctx, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin()));
  MutationEngine Engine;
  Engine.applyMutation(Testee->getParent(), *MP);

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

  ASSERT_EQ(ExecutionStatus::Failed, Runner.runTest(Test.get(), ObjectFiles).Status);

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());
}

TEST(SimpleTestRunner, runTestUsingLibC) {
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

  auto OwnedModuleWithTests   = TestModuleFactory.createLibCTesterModule();
  auto OwnedModuleWithTestees = TestModuleFactory.createLibCTesteeModule();

  Module *ModuleWithTests   = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  auto mutangOwnedModuleWithTests   = make_unique<MutangModule>(std::move(OwnedModuleWithTests), "");
  auto mutangOwnedModuleWithTestees = make_unique<MutangModule>(std::move(OwnedModuleWithTestees), "");

  Ctx.addModule(std::move(mutangOwnedModuleWithTests));
  Ctx.addModule(std::move(mutangOwnedModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto Tests = Finder.findTests(Ctx);

  ASSERT_NE(0U, Tests.size());

  auto &Test = *(Tests.begin());

  auto Obj = compiler.compileModule(ModuleWithTests);
  ObjectFiles.push_back(Obj.getBinary());
  OwnedObjectFiles.push_back(std::move(Obj));

  Obj = compiler.compileModule(ModuleWithTestees);
  ObjectFiles.push_back(Obj.getBinary());
  OwnedObjectFiles.push_back(std::move(Obj));

  /// Here we run test with original testee function
  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get(), ObjectFiles).Status);

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);
  ASSERT_NE(0U, Testees.size());
  Function *Testee = Testees[1]->getTesteeFunction();

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin()));
  MutationEngine Engine;
  Engine.applyMutation(Testee->getParent(), *MP);

  Obj = compiler.compileModule(ModuleWithTests);
  ObjectFiles.push_back(Obj.getBinary());
  OwnedObjectFiles.push_back(std::move(Obj));

  Obj = compiler.compileModule(ModuleWithTestees);
  ObjectFiles.push_back(Obj.getBinary());
  OwnedObjectFiles.push_back(std::move(Obj));

  ASSERT_EQ(ExecutionStatus::Failed, Runner.runTest(Test.get(), ObjectFiles).Status);
  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());
}

TEST(SimpleTestRunner, runTestUsingExternalLibrary) {
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

  /// No mutations applied here, since the only point of interest
  /// is an external libraries, in this case it is 'sqlite3'
  auto OwnedModuleWithTests   = TestModuleFactory.createExternalLibTesterModule();
  auto OwnedModuleWithTestees = TestModuleFactory.createExternalLibTesteeModule();

  Module *ModuleWithTests   = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  auto mutangOwnedModuleWithTests   = make_unique<MutangModule>(std::move(OwnedModuleWithTests), "");
  auto mutangOwnedModuleWithTestees = make_unique<MutangModule>(std::move(OwnedModuleWithTestees), "");

  Ctx.addModule(std::move(mutangOwnedModuleWithTests));
  Ctx.addModule(std::move(mutangOwnedModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  auto Tests = testFinder.findTests(Ctx);

  ASSERT_NE(0U, Tests.size());

  auto &Test = *(Tests.begin());

  llvm::sys::DynamicLibrary::LoadLibraryPermanently("/usr/lib/libsqlite3.dylib");

  auto Obj = compiler.compileModule(ModuleWithTestees);
  ObjectFiles.push_back(Obj.getBinary());
  OwnedObjectFiles.push_back(std::move(Obj));

  Obj = compiler.compileModule(ModuleWithTests);
  ObjectFiles.push_back(Obj.getBinary());
  OwnedObjectFiles.push_back(std::move(Obj));

  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get(), ObjectFiles).Status);
}
