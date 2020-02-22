#include "mull/Reporters/MutationTestingElementsReporter.h"

#include "FixturePaths.h"
#include "TestModuleFactory.h"

#include "mull/Bitcode.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/Metrics/Metrics.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include "mull/ReachableFunction.h"
#include "mull/Reporters/ASTSourceInfoProvider.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>

#include <fstream>
#include <gtest/gtest.h>
#include <json11/json11.hpp>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <ostream>

using namespace mull;
using namespace llvm;
using namespace json11;

static const int beginColumnStub = 1;
static const int beginLineStub = 2;
static const int endColumnStub = 3;
static const int endLineStub = 4;

class MockASTSourceInfoProvider : public SourceInfoProvider {
public:
  virtual ~MockASTSourceInfoProvider() = default;

  MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics,
                                        MutationPoint *mutationPoint) override {
    MutationPointSourceInfo info;
    info.beginColumn = beginColumnStub;
    info.beginLine = beginLineStub;
    info.endColumn = endColumnStub;
    info.endLine = endLineStub;
    return info;
  }
};

TEST(MutationTestingElementsReporterTest, integrationTest) {

  /// STEP 1. Long setup of:
  /// - 1 test with 1 testee with 1 mutation point.
  /// - 1 test execution result which includes 1 normal test execution and 1
  /// mutated test execution.
  Diagnostics diagnostics;
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(), llvmContext, diagnostics);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext, diagnostics);

  std::vector<std::unique_ptr<Bitcode>> modules;
  modules.push_back(std::move(bitcodeWithTests));
  modules.push_back(std::move(bitcodeWithTestees));
  Program program({}, {}, std::move(modules));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<cxx::AddToSub>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);

  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(program);

  auto &test = tests.front();

  Function *reachableFunction = program.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(reachableFunction->empty());

  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  reachableFunctions.emplace_back(make_unique<ReachableFunction>(reachableFunction, nullptr, 1));
  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);
  functionsUnderTest.back().selectInstructions({});

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, program, functionsUnderTest);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();

  std::vector<std::string> testIds({ test.getUniqueIdentifier(), test.getUniqueIdentifier() });
  std::vector<std::string> mutationPointIds({ "", mutationPoint->getUniqueIdentifier() });

  const long long RunningTime_1 = 1;
  const long long RunningTime_2 = 2;

  ExecutionResult testExecutionResult;
  testExecutionResult.status = Passed;
  testExecutionResult.runningTime = RunningTime_1;
  testExecutionResult.stdoutOutput = "testExecutionResult.STDOUT";
  testExecutionResult.stderrOutput = "testExecutionResult.STDERR";

  test.setExecutionResult(testExecutionResult);

  ExecutionResult mutatedTestExecutionResult;
  mutatedTestExecutionResult.status = Failed;
  mutatedTestExecutionResult.runningTime = RunningTime_2;
  mutatedTestExecutionResult.stdoutOutput = "mutatedTestExecutionResult.STDOUT";
  mutatedTestExecutionResult.stderrOutput = "mutatedTestExecutionResult.STDERR";

  auto mutationResult = make_unique<MutationResult>(
      mutatedTestExecutionResult, mutationPoint, reachableFunctions.front()->getDistance(), &test);

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  mutationResults.push_back(std::move(mutationResult));

  MetricsMeasure resultTime;

  Result result(std::move(tests), std::move(mutationResults), mutationPoints);

  /// STEP2. Reporting results to JSON
  MockASTSourceInfoProvider sourceInfoProvider;
  MutationTestingElementsReporter reporter(diagnostics, "", "", sourceInfoProvider);
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, metrics);

  /// STEP3. Making assertions.
  std::vector<ExecutionResult> executionResults{ testExecutionResult, mutatedTestExecutionResult };

  std::ifstream t(reporter.getJSONPath());
  std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  std::string err;
  Json object = Json::parse(str, err);

  std::cout << object.dump() << '\n';
  ASSERT_FALSE(object.is_null());

  const std::map<std::string, Json> &filesJSON = object["files"].object_items();
  ASSERT_EQ(1, filesJSON.size());

  const Json &fileJSON = filesJSON.begin()->second;

  const std::vector<Json> &mutantsJSON = fileJSON["mutants"].array_items();
  ASSERT_EQ(1, mutantsJSON.size());

  const Json &mutationJSON = mutantsJSON.at(0);

  const std::string &mutationId = mutationJSON["id"].string_value();
  ASSERT_EQ("cxx_add_to_sub", mutationId);

  const std::string &mutationStatus = mutationJSON["status"].string_value();
  ASSERT_EQ("Killed", mutationStatus);

  const Json &locationJSON = mutationJSON["location"].object_items();

  const Json &startLocationJSON = locationJSON["start"].object_items();
  const Json &endLocationJSON = locationJSON["end"].object_items();

  const int &startLine = startLocationJSON["line"].int_value();
  ASSERT_EQ(beginLineStub, startLine);

  const int &startColumn = startLocationJSON["column"].int_value();
  ASSERT_EQ(beginColumnStub, startColumn);

  const int &endLine = endLocationJSON["line"].int_value();
  ASSERT_EQ(endLineStub, endLine);

  const int &endColumn = endLocationJSON["column"].int_value();
  ASSERT_EQ(endColumnStub, endColumn);
}
