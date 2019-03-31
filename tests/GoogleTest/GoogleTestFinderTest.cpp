#include "mull/TestFrameworks/GoogleTest/GoogleTestFinder.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/ConfigParser.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/Filter.h"
#include "mull/ModuleLoader.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/GoogleTest/GoogleTestRunner.h"
#include "mull/TestFrameworks/GoogleTest/GoogleTest_Test.h"
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
  ModuleLoader loader;
  auto ModuleWithTests = loader.loadModuleAtPath(
      fixtures::google_test_google_test_Test_bc_path(), llvmContext);

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(ModuleWithTests));
  Program program({}, {}, std::move(modules));

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

  GoogleTest_Test *Test1 = dyn_cast<GoogleTest_Test>(tests[0].get());
  ASSERT_EQ("HelloTest.testSumOfTestee", Test1->getTestName());

  GoogleTest_Test *Test2 = dyn_cast<GoogleTest_Test>(tests[1].get());
  ASSERT_EQ("HelloTest.testSumOfTestee2", Test2->getTestName());
}

TEST(GoogleTestFinder, findTests_filter) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto ModuleWithTests = loader.loadModuleAtPath(
      fixtures::google_test_google_test_Test_bc_path(), llvmContext);

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(ModuleWithTests));
  Program program({}, {}, std::move(modules));

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
  GoogleTest_Test *Test1 = dyn_cast<GoogleTest_Test>(tests[0].get());
  ASSERT_EQ("HelloTest.testSumOfTestee", Test1->getTestName());
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
  ModuleLoader loader;
  auto moduleWithTests = loader.loadModuleAtPath(
      fixtures::google_test_google_test_Test_bc_path(), llvmContext);
  auto moduleWithTestees = loader.loadModuleAtPath(
      fixtures::google_test_google_test_Testee_bc_path(), llvmContext);

  auto compiledModule_tests = toolchain.compiler().compileModule(
      moduleWithTests->getModule(), toolchain.targetMachine());
  auto compiledModule_testees = toolchain.compiler().compileModule(
      moduleWithTestees->getModule(), toolchain.targetMachine());

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(moduleWithTests));
  modules.push_back(std::move(moduleWithTestees));
  Program program({}, {}, std::move(modules));

  Filter filter;
  GoogleTestFinder Finder;

  auto tests = Finder.findTests(program, filter);

  ASSERT_EQ(2U, tests.size());

  GoogleTest_Test *Test1 = dyn_cast<GoogleTest_Test>(tests[0].get());
  ASSERT_EQ("HelloTest.testSumOfTestee", Test1->getTestName());

  GoogleTest_Test *Test2 = dyn_cast<GoogleTest_Test>(tests[1].get());
  ASSERT_EQ("HelloTest.testSumOfTestee2", Test2->getTestName());

  GoogleTestRunner runner(toolchain.mangler());
  JITEngine jit;

  std::vector<llvm::object::ObjectFile *> objects(
      {compiledModule_tests.getBinary(), compiledModule_testees.getBinary()});
  /// TODO: enable eventually
  //  runner.loadProgram(objects, jit);
  runner.runTest(Test1, jit);
}
