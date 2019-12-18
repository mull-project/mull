#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include "mull/ReachableFunction.h"
#include "mull/TestFrameworks/NativeTestRunner.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Toolchain.h"
#include "mull/Toolchain/Trampolines.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>
#include <mull/Parallelization/Parallelization.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <gtest/gtest.h>
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;
using namespace llvm;

TEST(NativeTestRunner, runTest) {
  Diagnostics diagnostics;
  Configuration configuration;

  Toolchain toolchain(diagnostics, configuration);

  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(), context, diagnostics);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), context, diagnostics);

  Module *moduleWithTests = bitcodeWithTests->getModule();
  Module *moduleWithTestees = bitcodeWithTestees->getModule();

  std::vector<std::unique_ptr<Bitcode>> bitcodeFiles;
  bitcodeFiles.push_back(std::move(bitcodeWithTestees));
  bitcodeFiles.push_back(std::move(bitcodeWithTests));
  Program program({}, {}, std::move(bitcodeFiles));

  NativeTestRunner testRunner(diagnostics, toolchain.mangler());
  NativeTestRunner::ObjectFiles objectFiles;
  NativeTestRunner::OwnedObjectFiles ownedObjectFiles;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<cxx::AddToSub>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);

  Function *reachableFunction = program.lookupDefinedFunction("count_letters");
  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  reachableFunctions.emplace_back(make_unique<ReachableFunction>(reachableFunction, nullptr, 1));
  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);
  functionsUnderTest.back().selectInstructions({});

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, program, functionsUnderTest);
  for (auto point : mutationPoints) {
    point->getBitcode()->addMutation(point);
  }
  MutationPoint *mutationPoint = mutationPoints.front();

  SimpleTestFinder testFinder;

  auto tests = testFinder.findTests(program);

  ASSERT_NE(0U, tests.size());

  auto &test = tests.front();

  JITEngine jit(diagnostics);

  Bitcode &bitcode = *mutationPoint->getBitcode();
  std::vector<std::string> mutatedFunctions;
  CloneMutatedFunctionsTask::cloneFunctions(bitcode, mutatedFunctions);
  DeleteOriginalFunctionsTask::deleteFunctions(bitcode);
  InsertMutationTrampolinesTask::insertTrampolines(bitcode);
  mutationPoint->applyMutation();

  {
    auto owningBinary =
        toolchain.compiler().compileModule(moduleWithTests, toolchain.targetMachine());
    objectFiles.push_back(owningBinary.getBinary());
    ownedObjectFiles.push_back(std::move(owningBinary));
  }

  {
    auto owningBinary =
        toolchain.compiler().compileModule(moduleWithTestees, toolchain.targetMachine());
    objectFiles.push_back(owningBinary.getBinary());
    ownedObjectFiles.push_back(std::move(owningBinary));
  }

  Trampolines trampolines(mutatedFunctions);

  testRunner.loadMutatedProgram(objectFiles, trampolines, jit);
  trampolines.fixupOriginalFunctions(jit);
  ASSERT_EQ(ExecutionStatus::Passed, testRunner.runTest(jit, program, test));

  auto &mangler = toolchain.mangler();

  auto name = mutationPoint->getOriginalFunction()->getName().str();
  auto bitcodeId = mutationPoint->getBitcode()->getUniqueIdentifier();
  auto trampolineName = mangler.getNameWithPrefix(mutationPoint->getTrampolineName());
  auto mutatedFunctionName = mangler.getNameWithPrefix(mutationPoint->getMutatedFunctionName());
  uint64_t *trampoline = trampolines.findTrampoline(trampolineName);
  uint64_t address = llvm_compat::JITSymbolAddress(jit.getSymbol(mutatedFunctionName));
  assert(address);
  *trampoline = address;

  ASSERT_EQ(ExecutionStatus::Failed, testRunner.runTest(jit, program, test));
}
