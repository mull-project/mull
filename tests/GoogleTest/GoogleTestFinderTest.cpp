#include "mull/TestFrameworks/GoogleTest/GoogleTestFinder.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/NativeTestRunner.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/YAMLTraits.h>

#include <gtest/gtest.h>
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;
using namespace llvm;

#pragma mark - Finding Tests

TEST(GoogleTestFinder, FindTest) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Test_bc_path(), context, diagnostics);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  Program program({}, {}, std::move(bitcode));

  GoogleTestFinder finder;

  auto tests = finder.findTests(program);

  ASSERT_EQ(2U, tests.size());

  ASSERT_EQ("HelloTest.testSumOfTestee", tests[0].getTestName());
  ASSERT_EQ("HelloTest.testSumOfTestee2", tests[1].getTestName());
}

TEST(DISABLED_GoogleTestRunner, runTest) {
  Diagnostics diagnostics;
  Configuration configuration;

  Toolchain toolchain(diagnostics, configuration);

  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Test_bc_path(), context, diagnostics);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Testee_bc_path(), context, diagnostics);

  auto binaryWithTests =
      toolchain.compiler().compileModule(bitcodeWithTests->getModule(), toolchain.targetMachine());
  auto binaryWithTestees = toolchain.compiler().compileModule(bitcodeWithTestees->getModule(),
                                                              toolchain.targetMachine());

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  bitcode.push_back(std::move(bitcodeWithTestees));
  Program program({}, {}, std::move(bitcode));

  GoogleTestFinder Finder;

  auto tests = Finder.findTests(program);

  ASSERT_EQ(2U, tests.size());

  ASSERT_EQ("HelloTest.testSumOfTestee", tests[0].getTestName());
  ASSERT_EQ("HelloTest.testSumOfTestee2", tests[1].getTestName());

  NativeTestRunner runner(diagnostics, toolchain.mangler());
  JITEngine jit(diagnostics);

  std::vector<llvm::object::ObjectFile *> objects(
      { binaryWithTests.getBinary(), binaryWithTestees.getBinary() });
  /// TODO: enable eventually
  //  runner.loadProgram(objects, jit);
  runner.runTest(jit, program, tests[0]);
}
