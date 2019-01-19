#include "Context.h"
#include "Mutators/MathAddMutator.h"
#include "TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "TestFrameworks/SimpleTest/SimpleTestRunner.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "MutationsFinder.h"
#include "Filter.h"
#include "Testee.h"
#include "Toolchain/Toolchain.h"
#include "Toolchain/JITEngine.h"
#include "Toolchain/Trampolines.h"
#include "Config/Configuration.h"
#include "FixturePaths.h"

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

TEST(SimpleTestRunner, runTest) {
  Configuration configuration;

  Toolchain toolchain(configuration);

  LLVMContext llvmContext;
  ModuleLoader loader;
  auto ownedModuleWithTests = loader.loadModuleAtPath(fixtures::simple_test_count_letters_test_count_letters_bc_path(), llvmContext);
  auto ownedModuleWithTestees = loader.loadModuleAtPath(fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext);

  Module *moduleWithTests   = ownedModuleWithTests->getModule();
  Module *moduleWithTestees = ownedModuleWithTestees->getModule();

  Context context;
  SimpleTestRunner testRunner(toolchain.mangler());
  SimpleTestRunner::ObjectFiles objectFiles;
  SimpleTestRunner::OwnedObjectFiles ownedObjectFiles;

  context.addModule(std::move(ownedModuleWithTests));
  context.addModule(std::move(ownedModuleWithTestees));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);
  Filter filter;

  Function *testeeFunction = context.lookupDefinedFunction("count_letters");
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(context, mergedTestees, filter);

  MutationPoint *mutationPoint = mutationPoints.front();

  SimpleTestFinder testFinder;

  auto tests = testFinder.findTests(context, filter);

  ASSERT_NE(0U, tests.size());

  auto &test = tests.front();

  JITEngine jit;

  auto mutatedFunctions = mutationPoint->getOriginalModule()->prepareMutations();
  mutationPoint->applyMutation();

  {
    auto owningBinary = toolchain.compiler().compileModule(moduleWithTests, toolchain.targetMachine());
    objectFiles.push_back(owningBinary.getBinary());
    ownedObjectFiles.push_back(std::move(owningBinary));
  }

  {
    auto owningBinary = toolchain.compiler().compileModule(moduleWithTestees, toolchain.targetMachine());
    objectFiles.push_back(owningBinary.getBinary());
    ownedObjectFiles.push_back(std::move(owningBinary));
  }

  Trampolines trampolines(mutatedFunctions);

  testRunner.loadMutatedProgram(objectFiles, trampolines, jit);
  trampolines.fixupOriginalFunctions(jit);
  ASSERT_EQ(ExecutionStatus::Passed, testRunner.runTest(test.get(), jit));

  auto &mangler = toolchain.mangler();

  auto name = mutationPoint->getOriginalFunction()->getName().str();
  auto moduleId = mutationPoint->getOriginalModule()->getUniqueIdentifier();
  auto trampolineName = mangler.getNameWithPrefix(mutationPoint->getTrampolineName());
  auto mutatedFunctionName = mangler.getNameWithPrefix(mutationPoint->getMutatedFunctionName());
  uint64_t *trampoline = trampolines.findTrampoline(trampolineName);
  uint64_t address = llvm_compat::JITSymbolAddress(jit.getSymbol(mutatedFunctionName));
  assert(address);
  *trampoline = address;

  ASSERT_EQ(ExecutionStatus::Failed, testRunner.runTest(test.get(), jit));
}

