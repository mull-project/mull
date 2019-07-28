#include "mull/TestFrameworks/GoogleTest/GoogleTestFinder.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/ConfigParser.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/Filter.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/NativeTestRunner.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLTraits.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

#pragma mark - Finding Tests

TEST(GoogleTestFinder, FindTest) {
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Test_bc_path(), llvmContext);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  Program program({}, {}, std::move(bitcode));

  const char *configYAML = R"YAML(
mutators:
- math_add_mutator
- negate_mutator
- remove_void_function_mutator
)YAML";

  yaml::Input Input(configYAML);

  ConfigParser Parser;
  auto config = Parser.loadConfig(Input);

  Filter filter;
  GoogleTestFinder Finder;

  auto tests = Finder.findTests(program, filter);

  ASSERT_EQ(2U, tests.size());

  ASSERT_EQ("HelloTest.testSumOfTestee", tests[0].getTestName());
  ASSERT_EQ("HelloTest.testSumOfTestee2", tests[1].getTestName());
}

TEST(GoogleTestFinder, findTests_filter) {
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Test_bc_path(), llvmContext);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  Program program({}, {}, std::move(bitcode));

  const char *configYAML = R"YAML(
mutators:
  - math_add_mutator
  - negate_mutator
  - remove_void_function_mutator
  )YAML";

  yaml::Input Input(configYAML);

  ConfigParser Parser;
  auto config = Parser.loadConfig(Input);

  Filter filter;
  filter.includeTest("HelloTest.testSumOfTestee");
  GoogleTestFinder Finder;

  auto tests = Finder.findTests(program, filter);

  ASSERT_EQ(1U, tests.size());
  ASSERT_EQ("HelloTest.testSumOfTestee", tests[0].getTestName());
}

TEST(DISABLED_GoogleTestRunner, runTest) {
  const char *configYAML = R"YAML(
mutators:
  - math_add_mutator
  - negate_mutator
  - remove_void_function_mutator
  )YAML";

  yaml::Input Input(configYAML);

  ConfigParser Parser;
  auto rawConfig = Parser.loadConfig(Input);
  Configuration configuration(rawConfig);

  Toolchain toolchain(configuration);

  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Test_bc_path(), llvmContext);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Testee_bc_path(), llvmContext);

  auto binaryWithTests = toolchain.compiler().compileModule(
      bitcodeWithTests->getModule(), toolchain.targetMachine());
  auto binaryWithTestees = toolchain.compiler().compileModule(
      bitcodeWithTestees->getModule(), toolchain.targetMachine());

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  bitcode.push_back(std::move(bitcodeWithTestees));
  Program program({}, {}, std::move(bitcode));

  Filter filter;
  GoogleTestFinder Finder;

  auto tests = Finder.findTests(program, filter);

  ASSERT_EQ(2U, tests.size());

  ASSERT_EQ("HelloTest.testSumOfTestee", tests[0].getTestName());
  ASSERT_EQ("HelloTest.testSumOfTestee2", tests[1].getTestName());

  NativeTestRunner runner(toolchain.mangler());
  JITEngine jit;

  std::vector<llvm::object::ObjectFile *> objects(
      {binaryWithTests.getBinary(), binaryWithTestees.getBinary()});
  /// TODO: enable eventually
  //  runner.loadProgram(objects, jit);
  runner.runTest(jit, program, tests[0]);
}
