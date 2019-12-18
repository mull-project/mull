#include "mull/Reporters/SQLiteReporter.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Metrics/Metrics.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include "mull/ReachableFunction.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>

#include <cstring>
#include <gtest/gtest.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <sqlite3.h>

using namespace mull;
using namespace llvm;

TEST(SQLiteReporter, integrationTest) {

  /// STEP 1. Long setup of:
  /// - 1 test with 1 testee with 1 mutation point.
  /// - 1 test execution result which includes 1 normal test execution and 1
  /// mutated test execution.
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(), context, diagnostics);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), context, diagnostics);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  bitcode.push_back(std::move(bitcodeWithTestees));
  Program program({}, {}, std::move(bitcode));
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

  /// STEP2. Reporting results to SQLite
  SQLiteReporter reporter(diagnostics, "integration test", "");
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, metrics);

  /// STEP3. Making assertions.
  std::vector<ExecutionResult> executionResults{ testExecutionResult, mutatedTestExecutionResult };

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  {
    std::string selectQuery = "SELECT * FROM execution_result";
    sqlite3_stmt *selectStmt;
    sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, nullptr);

    const unsigned char *column_test_id;
    const unsigned char *column_mutation_point_id;
    int column_status;
    int column_duration;
    const unsigned char *column_stdout;
    const unsigned char *column_stderr;

    int numberOfRows = 0;
    while (1) {
      int stepResult = sqlite3_step(selectStmt);

      if (stepResult == SQLITE_ROW) {
        column_test_id = sqlite3_column_text(selectStmt, 0);
        column_mutation_point_id = sqlite3_column_text(selectStmt, 1);

        column_status = sqlite3_column_int(selectStmt, 2);
        column_duration = sqlite3_column_int(selectStmt, 3);

        column_stdout = sqlite3_column_text(selectStmt, 4);
        column_stderr = sqlite3_column_text(selectStmt, 5);

        ASSERT_EQ(strcmp((const char *)column_test_id, testIds[numberOfRows].c_str()), 0);
        ASSERT_EQ(
            strcmp((const char *)column_mutation_point_id, mutationPointIds[numberOfRows].c_str()),
            0);

        ASSERT_EQ(column_status, executionResults[numberOfRows].status);
        ASSERT_EQ(column_duration, executionResults[numberOfRows].runningTime);

        ASSERT_EQ(strcmp((const char *)column_stdout,
                         executionResults[numberOfRows].stdoutOutput.c_str()),
                  0);
        ASSERT_EQ(strcmp((const char *)column_stderr,
                         executionResults[numberOfRows].stderrOutput.c_str()),
                  0);

        numberOfRows++;
      } else if (stepResult == SQLITE_DONE) {
        break;
      } else {
        fprintf(stderr, "Failed.\n");
        exit(1);
      }
    }

    ASSERT_EQ(numberOfRows, 2);

    sqlite3_finalize(selectStmt);
  }

  {
    std::string selectQuery = "SELECT * FROM test";
    sqlite3_stmt *selectStmt;
    sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, nullptr);

    const unsigned char *test_name;
    const unsigned char *test_unique_id;
    const unsigned char *test_location_file;
    int test_location_line;

    int numberOfRows = 0;
    while (1) {
      int stepResult = sqlite3_step(selectStmt);

      if (stepResult == SQLITE_ROW) {
        int column = 0;
        test_name = sqlite3_column_text(selectStmt, column++);
        test_unique_id = sqlite3_column_text(selectStmt, column++);
        test_location_file = sqlite3_column_text(selectStmt, column++);
        test_location_line = sqlite3_column_int(selectStmt, column++);

        ASSERT_EQ(strcmp((const char *)test_name, "test_count_letters"), 0);
        ASSERT_EQ(strcmp((const char *)test_unique_id, "test_count_letters"), 0);

        const char *location = "simple_test/count_letters/test_count_letters.c";
        ASSERT_NE(strstr((const char *)test_location_file, location), nullptr);

        ASSERT_EQ(test_location_line, 5);

        numberOfRows++;
      } else if (stepResult == SQLITE_DONE) {
        break;
      } else {
        fprintf(stderr, "Failed.\n");
        exit(1);
      }
    }

    ASSERT_EQ(numberOfRows, 1);

    sqlite3_finalize(selectStmt);
  }

  sqlite3_close(database);
}
