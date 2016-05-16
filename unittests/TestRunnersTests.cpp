#include "MutationEngine.h"
#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
#include "TestModuleFactory.h"
#include "TestFinders/SimpleTestFinder.h"
#include "TestRunners/SimpleTestRunner.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestRunner, runTest) {
  auto OwnedModuleWithTests   = TestModuleFactory.createTesterModule();
  auto OwnedModuleWithTestees = TestModuleFactory.createTesteeModule();

  Module *ModuleWithTests   = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  Context Ctx;
  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  ASSERT_NE(0U, Tests.size());

  Function *Test = *(Tests.begin());

  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests);

  SimpleTestRunner Runner(Modules);

  /// Here we run test with original testee function
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees));

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);
  ASSERT_NE(0U, Testees.size());
  Function *Testee = *(Testees.begin());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  MutationEngine Engine;
  Engine.applyMutation(*MP);

  ASSERT_EQ(Failed, Runner.runTest(Test, ModuleWithTestees));
}

TEST(SimpleTestRunner, runTestUsingLibC) {
  auto OwnedModuleWithTests   = TestModuleFactory.createLibCTesterModule();
  auto OwnedModuleWithTestees = TestModuleFactory.createLibCTesteeModule();

  Module *ModuleWithTests   = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  Context Ctx;
  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  ASSERT_NE(0U, Tests.size());

  Function *Test = *(Tests.begin());

  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests);

  SimpleTestRunner Runner(Modules);

  /// Here we run test with original testee function
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees));

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);
  ASSERT_NE(0U, Testees.size());
  Function *Testee = *(Testees.begin());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  MutationEngine Engine;
  Engine.applyMutation(*MP);

  ASSERT_EQ(Failed, Runner.runTest(Test, ModuleWithTestees));
}

TEST(SimpleTestRunner, runTestUsingExternalLibrary) {
  /// No mutations applied here, since the only point of interest
  /// is an external libraries, in this case it is 'sqlite3'
  auto OwnedModuleWithTests   = TestModuleFactory.createExternalLibTesterModule();
  auto OwnedModuleWithTestees = TestModuleFactory.createExternalLibTesteeModule();

  Module *ModuleWithTests   = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  Context Ctx;
  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();
  
  ASSERT_NE(0U, Tests.size());
  
  Function *Test = *(Tests.begin());
  
  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests);
  
  SimpleTestRunner Runner(Modules);
  
  llvm::sys::DynamicLibrary::LoadLibraryPermanently("/usr/lib/libsqlite3.dylib");
  
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees));
}
