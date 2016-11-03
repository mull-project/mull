#include "SQLiteReporter.h"
#include "TestResult.h"
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

  ExecutionResult result;
  result.Status = Passed;
  result.RunningTime = RunningTime;
  result.stdoutOutput = "STDOUT";
  result.stderrOutput = "STDERR";

  LLVMContext Context;

  std::unique_ptr<Module> moduleOwner(new Module("test", Context));
  Module *module = moduleOwner.get();

  Function *function =
    cast<Function>(module->getOrInsertFunction("fib", Type::getInt32Ty(Context),
                                          Type::getInt32Ty(Context),
                                          nullptr));

  std::unique_ptr<SimpleTest_Test> test = make_unique<SimpleTest_Test>(function);

  std::unique_ptr<TestResult> testResult = make_unique<TestResult>(result, std::move(test));
  std::vector<std::unique_ptr<TestResult>> results;
  results.push_back(std::move(testResult));

  reporter.reportResults(results);

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
                       result.stdoutOutput.c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column4_stderr,
                       result.stderrOutput.c_str()), 0);

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
