
#include "Context.h"
#include "Mutators/MathAddMutator.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "MutationsFinder.h"
#include "Filter.h"
#include "Testee.h"
#include "Toolchain/Toolchain.h"

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

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestRunner, runTest) {
  Config config;
  config.normalizeParallelizationConfig();

  Toolchain toolchain(config);

  Context context;
  SimpleTestRunner runner(toolchain.mangler());
  SimpleTestRunner::ObjectFiles objectFiles;
  SimpleTestRunner::OwnedObjectFiles ownedObjectFiles;

  auto ownedModuleWithTests   = TestModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto ownedModuleWithTestees = TestModuleFactory.create_SimpleTest_CountLetters_Module();

  Module *moduleWithTests   = ownedModuleWithTests->getModule();
  Module *moduleWithTestees = ownedModuleWithTestees->getModule();

  context.addModule(std::move(ownedModuleWithTests));
  context.addModule(std::move(ownedModuleWithTestees));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder mutationsFinder(std::move(mutators), config);
  Filter filter;

  SimpleTestFinder testFinder;

  auto Tests = testFinder.findTests(context, filter);

  ASSERT_NE(0U, Tests.size());

  auto &Test = *(Tests.begin());

  {
    auto object = toolchain.compiler().compileModule(moduleWithTests, toolchain.targetMachine());
    objectFiles.push_back(object.getBinary());
    ownedObjectFiles.push_back(std::move(object));
  }

  {
    auto object = toolchain.compiler().compileModule(moduleWithTestees, toolchain.targetMachine());
    objectFiles.push_back(object.getBinary());
    ownedObjectFiles.push_back(std::move(object));
  }

  JITEngine jit;

  /// Here we run test with original testee function
  runner.loadProgram(objectFiles, jit);
  ASSERT_EQ(ExecutionStatus::Passed, runner.runTest(Test.get(), jit));

  objectFiles.erase(objectFiles.begin(), objectFiles.end());

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  Function *testeeFunction = context.lookupDefinedFunction("count_letters");
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> MutationPoints =
    mutationsFinder.getMutationPoints(context, mergedTestees, filter);

  MutationPoint *MP = (*(MutationPoints.begin()));

  LLVMContext localContext;
  auto ownedMutatedTesteeModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedTesteeModule);

  {
    auto object = toolchain.compiler().compileModule(moduleWithTests, toolchain.targetMachine());
    objectFiles.push_back(object.getBinary());
    ownedObjectFiles.push_back(std::move(object));
  }

  {
    auto object = toolchain.compiler().compileModule(ownedMutatedTesteeModule->getModule(), toolchain.targetMachine());
    objectFiles.push_back(object.getBinary());
    ownedObjectFiles.push_back(std::move(object));
  }

  runner.loadProgram(objectFiles, jit);
  ASSERT_EQ(ExecutionStatus::Failed, runner.runTest(Test.get(), jit));

  objectFiles.erase(objectFiles.begin(), objectFiles.end());
}
