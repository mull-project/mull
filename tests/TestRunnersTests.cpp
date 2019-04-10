#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/ModuleLoader.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MathAddMutator.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestRunner.h"
#include "mull/Testee.h"
#include "mull/Toolchain/Compiler.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Toolchain.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(SimpleTestRunner, runTest) {
  Configuration configuration;

  Toolchain toolchain(configuration);

  LLVMContext llvmContext;
  ModuleLoader loader;
  auto ownedModuleWithTests = loader.loadModuleAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);
  auto ownedModuleWithTestees = loader.loadModuleAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext);

  Module *moduleWithTests = ownedModuleWithTests->getModule();
  Module *moduleWithTestees = ownedModuleWithTestees->getModule();

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(ownedModuleWithTestees));
  modules.push_back(std::move(ownedModuleWithTests));
  Program program({}, {}, std::move(modules));

  SimpleTestRunner testRunner(toolchain.mangler());
  SimpleTestRunner::ObjectFiles objectFiles;
  SimpleTestRunner::OwnedObjectFiles ownedObjectFiles;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);
  Filter filter;

  Function *testeeFunction = program.lookupDefinedFunction("count_letters");
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(program, mergedTestees, filter);

  MutationPoint *mutationPoint = mutationPoints.front();

  SimpleTestFinder testFinder;

  auto tests = testFinder.findTests(program, filter);

  ASSERT_NE(0U, tests.size());

  auto &test = tests.front();

  JITEngine jit;

  auto mutatedFunctions =
      mutationPoint->getOriginalModule()->prepareMutations();
  mutationPoint->applyMutation();

  {
    auto owningBinary = toolchain.compiler().compileModule(
        moduleWithTests, toolchain.targetMachine());
    objectFiles.push_back(owningBinary.getBinary());
    ownedObjectFiles.push_back(std::move(owningBinary));
  }

  {
    auto owningBinary = toolchain.compiler().compileModule(
        moduleWithTestees, toolchain.targetMachine());
    objectFiles.push_back(owningBinary.getBinary());
    ownedObjectFiles.push_back(std::move(owningBinary));
  }

  Trampolines trampolines(mutatedFunctions);

  testRunner.loadMutatedProgram(objectFiles, trampolines, jit);
  trampolines.fixupOriginalFunctions(jit);
  ASSERT_EQ(ExecutionStatus::Passed, testRunner.runTest(jit, program, test));

  auto &mangler = toolchain.mangler();

  auto name = mutationPoint->getOriginalFunction()->getName().str();
  auto moduleId = mutationPoint->getOriginalModule()->getUniqueIdentifier();
  auto trampolineName =
      mangler.getNameWithPrefix(mutationPoint->getTrampolineName());
  auto mutatedFunctionName =
      mangler.getNameWithPrefix(mutationPoint->getMutatedFunctionName());
  uint64_t *trampoline = trampolines.findTrampoline(trampolineName);
  uint64_t address =
      llvm_compat::JITSymbolAddress(jit.getSymbol(mutatedFunctionName));
  assert(address);
  *trampoline = address;

  ASSERT_EQ(ExecutionStatus::Failed, testRunner.runTest(jit, program, test));
}
