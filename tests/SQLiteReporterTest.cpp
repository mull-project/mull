#include "mull/Reporters/SQLiteReporter.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/FunctionUnderTest.h"
#include "mull/Metrics/MetricsMeasure.h"
#include "mull/Mutant.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include "mull/Result.h"
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
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(), diagnostics);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), diagnostics);

  Function *reachableFunction = bitcodeWithTestees->getModule()->getFunction("count_letters");

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  bitcode.push_back(std::move(bitcodeWithTestees));
  Program program(std::move(bitcode));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::AddToSub>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);

  ASSERT_FALSE(reachableFunction->empty());

  std::vector<FunctionUnderTest> functionsUnderTest(
      { FunctionUnderTest(reachableFunction, program.bitcode().front().get()) });
  functionsUnderTest.back().selectInstructions({});
  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, program, functionsUnderTest);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();

  std::vector<std::string> mutationPointIds({ mutationPoint->getUserIdentifier() });

  const long long RunningTime_2 = 2;

  ExecutionResult mutatedTestExecutionResult;
  mutatedTestExecutionResult.status = Failed;
  mutatedTestExecutionResult.runningTime = RunningTime_2;
  mutatedTestExecutionResult.stdoutOutput = "mutatedTestExecutionResult.STDOUT";
  mutatedTestExecutionResult.stderrOutput = "mutatedTestExecutionResult.STDERR";

  auto anyPoint = mutationPoints.front();
  auto mutant = std::make_unique<Mutant>(mutationPoint->getUserIdentifier(),
                                         anyPoint->getMutatorIdentifier(),
                                         anyPoint->getSourceLocation(),
                                         anyPoint->isCovered());
  auto mutationResult = std::make_unique<MutationResult>(mutatedTestExecutionResult, mutant.get());

  std::vector<std::unique_ptr<Mutant>> mutants;
  mutants.push_back(std::move(mutant));

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  mutationResults.push_back(std::move(mutationResult));

  MetricsMeasure resultTime;

  Result result(std::move(mutants), std::move(mutationResults));

  /// STEP2. Reporting results to SQLite
  SQLiteReporter reporter(diagnostics, "integration test", "");
  reporter.reportResults(result);

  /// STEP3. Making assertions.
  std::vector<ExecutionResult> executionResults{ mutatedTestExecutionResult };

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  {
    std::string selectQuery = "SELECT * FROM execution_result";
    sqlite3_stmt *selectStmt;
    sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, nullptr);

    const unsigned char *column_mutation_point_id;
    int column_status;
    int column_duration;
    const unsigned char *column_stdout;
    const unsigned char *column_stderr;

    int numberOfRows = 0;
    while (1) {
      int stepResult = sqlite3_step(selectStmt);

      if (stepResult == SQLITE_ROW) {
        column_mutation_point_id = sqlite3_column_text(selectStmt, 0);

        column_status = sqlite3_column_int(selectStmt, 1);
        column_duration = sqlite3_column_int(selectStmt, 2);

        column_stdout = sqlite3_column_text(selectStmt, 3);
        column_stderr = sqlite3_column_text(selectStmt, 4);

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

    ASSERT_EQ(numberOfRows, 1);

    sqlite3_finalize(selectStmt);
  }

  sqlite3_close(database);
}
