#include "GoogleTest/GoogleTestFinder.h"

#include "Driver.h"
#include "Context.h"
#include "Config.h"
#include "ConfigParser.h"
#include "MutationOperators/MutationOperatorsFactory.h"
#include "TestModuleFactory.h"
#include "GoogleTest/GoogleTest_Test.h"
#include "Filter.h"
#include "MutationsFinder.h"

#include "llvm/IR/CallSite.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLTraits.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

#pragma mark - Finding Tests

TEST(GoogleTestFinder, FindTest) {
  auto ModuleWithTests     = TestModuleFactory.createGoogleTestTesterModule();
  auto mullModuleWithTests = make_unique<MullModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));

  const char *configYAML = R"YAML(
mutation_operators:
- add_mutation_operator
- negate_mutation_operator
- remove_void_function_mutation_operator
)YAML";
  
  yaml::Input Input(configYAML);
  
  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  Filter filter;
  GoogleTestFinder Finder;

  auto tests = Finder.findTests(Ctx, filter);

  ASSERT_EQ(2U, tests.size());

  GoogleTest_Test *Test1 = dyn_cast<GoogleTest_Test>(tests[0].get());
  ASSERT_EQ("HelloTest.testSumOfTestee", Test1->getTestName());

  GoogleTest_Test *Test2 = dyn_cast<GoogleTest_Test>(tests[1].get());
  ASSERT_EQ("HelloTest.testSumOfTestee2", Test2->getTestName());
}

TEST(GoogleTestFinder, findTests_filter) {
  auto ModuleWithTests       = TestModuleFactory.createGoogleTestTesterModule();
  auto mullModuleWithTests = make_unique<MullModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));

  const char *configYAML = R"YAML(
mutation_operators:
  - add_mutation_operator
  - negate_mutation_operator
  - remove_void_function_mutation_operator
  )YAML";

  yaml::Input Input(configYAML);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  Filter filter;
  filter.includeTest("HelloTest.testSumOfTestee");
  GoogleTestFinder Finder;

  auto tests = Finder.findTests(Ctx, filter);

  ASSERT_EQ(1U, tests.size());
  GoogleTest_Test *Test1 = dyn_cast<GoogleTest_Test>(tests[0].get());
  ASSERT_EQ("HelloTest.testSumOfTestee", Test1->getTestName());
}
