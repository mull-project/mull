#include "SQLiteReporter.h"
#include "Result.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "gtest/gtest.h"

#include <cstring>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include <sqlite3.h>

using namespace Mutang;
using namespace llvm;

TEST(SQLiteReporter, integrationTest) {
  SQLiteReporter reporter;

  const long long RunningTime = 1;

  ExecutionResult testExecutionResult;
  testExecutionResult.Status = Passed;
  testExecutionResult.RunningTime = RunningTime;
  testExecutionResult.stdoutOutput = "STDOUT";
  testExecutionResult.stderrOutput = "STDERR";

  LLVMContext Context;

  std::unique_ptr<Module> moduleOwner(new Module("test", Context));
  Module *module = moduleOwner.get();

  Function *function =
    cast<Function>(module->getOrInsertFunction("fib", Type::getInt32Ty(Context),
                                          Type::getInt32Ty(Context),
                                          nullptr));

  std::unique_ptr<SimpleTest_Test> test = make_unique<SimpleTest_Test>(function);

  std::unique_ptr<TestResult> testResult =
    make_unique<TestResult>(testExecutionResult, std::move(test));

  std::vector<std::unique_ptr<TestResult>> results;
  results.push_back(std::move(testResult));

  /// In this test we are not interested in testees.
  std::vector<std::unique_ptr<Testee>> testees;

  std::unique_ptr<Result> result = make_unique<Result>(std::move(results),
                                                       std::move(testees));
  reporter.reportResults(result);

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT * FROM execution_result";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, NULL);

  int column1_status;
  int column2_duration;
  const unsigned char * column3_stdout;
  const unsigned char * column4_stderr;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step (selectStmt);

    if (stepResult == SQLITE_ROW) {
      column1_status = sqlite3_column_int(selectStmt, 0);
      column2_duration = sqlite3_column_int(selectStmt, 1);

      column3_stdout  = sqlite3_column_text (selectStmt, 2);
      column4_stderr  = sqlite3_column_text (selectStmt, 3);

      ASSERT_EQ(column1_status, Passed);
      ASSERT_EQ(column2_duration, RunningTime);

      ASSERT_EQ(strcmp((const char *)column3_stdout,
                       testExecutionResult.stdoutOutput.c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column4_stderr,
                       testExecutionResult.stderrOutput.c_str()), 0);

      numberOfRows++;
    }
    else if (stepResult == SQLITE_DONE) {
      break;
    }
    else {
      fprintf (stderr, "Failed.\n");
      exit (1);
    }
  }

  ASSERT_EQ(numberOfRows, 1);

  sqlite3_finalize(selectStmt);
  sqlite3_close(database);
}

TEST(SQLiteReporter, callerPathToString) {
  std::vector<std::string> callerPath;

  callerPath.push_back("func1:12");
  callerPath.push_back("func2:14");
  callerPath.push_back("func3:16");

  SQLiteReporter reporter;

  std::string expectedCallerPathString = "func1:12\n  func2:14\n    func3:16";
  ASSERT_EQ(reporter.getCallerPathAsString(callerPath),
            expectedCallerPathString);
}
