#include "Config.h"
#include "SQLiteReporter.h"
#include "Result.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "gtest/gtest.h"

#include <cstring>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include <sqlite3.h>

using namespace mull;
using namespace llvm;

TEST(SQLiteReporter, integrationTest) {
  SQLiteReporter reporter;

  const long long RunningTime = 1;

  ExecutionResult testExecutionResult;
  testExecutionResult.Status = Passed;
  testExecutionResult.RunningTime = RunningTime;
  testExecutionResult.stdoutOutput = "STDOUT";
  testExecutionResult.stderrOutput = "STDERR";
  
  ExecutionResult llvmErrorTestExecutionResult;
  llvmErrorTestExecutionResult.Status = Failed;
  llvmErrorTestExecutionResult.RunningTime = RunningTime;
  llvmErrorTestExecutionResult.stdoutOutput = "STDOUT";
  llvmErrorTestExecutionResult.stderrOutput = "LLVM ERROR: Program used external function '__ZTIN4llvm20SectionMemoryManagerE' which could not be resolved!";
  
  std::vector<ExecutionResult> executionResults { testExecutionResult, llvmErrorTestExecutionResult };

  LLVMContext Context;

  std::unique_ptr<Module> moduleOwner(new Module("test", Context));
  Module *module = moduleOwner.get();

  Function *function =
    cast<Function>(module->getOrInsertFunction("fib", Type::getInt32Ty(Context),
                                          Type::getInt32Ty(Context),
                                          nullptr));

  std::unique_ptr<SimpleTest_Test> test = make_unique<SimpleTest_Test>(function);
  std::unique_ptr<SimpleTest_Test> llvmErrorTest = make_unique<SimpleTest_Test>(function);

  std::unique_ptr<TestResult> testResult =
    make_unique<TestResult>(testExecutionResult, std::move(test));
  std::unique_ptr<TestResult> llvmErrorTestResult =
    make_unique<TestResult>(llvmErrorTestExecutionResult, std::move(llvmErrorTest));

  std::vector<std::unique_ptr<TestResult>> results;
  results.push_back(std::move(testResult));
  results.push_back(std::move(llvmErrorTestResult));

  /// In this test we are not interested in testees.
  std::vector<std::unique_ptr<Testee>> testees;

  ResultTime resultTime = {
    .start = 1234,
    .end = 5678
  };

  std::unique_ptr<Result> result = make_unique<Result>(std::move(results),
                                                       std::move(testees));
  reporter.reportResults(result, Config(), resultTime);

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

      ASSERT_EQ(column1_status, executionResults[numberOfRows].Status);
      ASSERT_EQ(column2_duration, executionResults[numberOfRows].RunningTime);

      ASSERT_EQ(strcmp((const char *)column3_stdout,
                       executionResults[numberOfRows].stdoutOutput.c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column4_stderr,
                       executionResults[numberOfRows].stderrOutput.c_str()), 0);

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

  ASSERT_EQ(numberOfRows, 2);

  sqlite3_finalize(selectStmt);
  sqlite3_close(database);
}

TEST(SQLiteReporter, integrationTest_Config) {
  std::string projectName("Integration Test");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList = "/tmp/dynamic_library_file_list.txt";

  std::ofstream bitcodeFile(bitcodeFileList);
  std::ofstream dynamicLibraryFile(dynamicLibraryFileList);

  if (!bitcodeFile) {
    std::cerr << "Cannot open the output file." << std::endl;

    ASSERT_FALSE(true);
  }

  bitcodeFile << "tester.bc" << std::endl;
  bitcodeFile << "testee.bc" << std::endl;
  
  if (!dynamicLibraryFile) {
    std::cerr << "Cannot open the output file." << std::endl;
    
    ASSERT_FALSE(true);
  }
  
  dynamicLibraryFile << "sqlite3.dylib" << std::endl;
  dynamicLibraryFile << "libz.dylib" << std::endl;

  std::vector<std::string> operators({
    "add_mutation",
    "negate_condition"
  });

  std::vector<std::string> tests({
    "test_method1",
    "test_method2"
  });


  bool doFork = true;
  bool dryRun = true;
  bool useCache = true;
  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  Config config(bitcodeFileList,
                projectName,
                testFramework,
                operators,
                dynamicLibraryFileList,
                tests,
                doFork, dryRun, useCache, timeout, distance,
                cacheDirectory);

  SQLiteReporter reporter(config.getProjectName());

  std::vector<std::unique_ptr<TestResult>> testResults;
  std::vector<std::unique_ptr<Testee>> allTestees;

  ResultTime resultTime = {
    .start = 1234,
    .end = 5678
  };

  std::unique_ptr<Result> result = make_unique<Result>(std::move(testResults),
                                                       std::move(allTestees));
  reporter.reportResults(result, config, resultTime);

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT * FROM config";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, NULL);

  const unsigned char *column1_projectName = nullptr;
  const unsigned char *column2_bitcodePaths = nullptr;
  const unsigned char *column3_operators = nullptr;
  const unsigned char *column4_dylibs = nullptr;
  const unsigned char *column5_tests = nullptr;

  int column6_fork = 0;
  int column7_dryRun = 0;
  int column8_useCache = 0;
  int column9_timeout = 0;
  int column10_distance = 0;
  const unsigned char *column11_cacheDirectory = nullptr;
  int column12_timeStart = 0;
  int column13_timeEnd = 0;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step (selectStmt);

    if (stepResult == SQLITE_ROW) {
      column1_projectName = sqlite3_column_text(selectStmt, 0);
      column2_bitcodePaths = sqlite3_column_text(selectStmt, 1);
      column3_operators = sqlite3_column_text(selectStmt, 2);
      column4_dylibs = sqlite3_column_text(selectStmt, 3);
      column5_tests = sqlite3_column_text(selectStmt, 4);
      column6_fork = sqlite3_column_int(selectStmt, 5);
      column7_dryRun = sqlite3_column_int(selectStmt, 6);
      column8_useCache = sqlite3_column_int(selectStmt, 7);
      column9_timeout = sqlite3_column_int(selectStmt, 8);
      column10_distance = sqlite3_column_int(selectStmt, 9);
      column11_cacheDirectory = sqlite3_column_text(selectStmt, 10);
      column12_timeStart = sqlite3_column_int(selectStmt, 11);
      column13_timeEnd = sqlite3_column_int(selectStmt, 12);

      ASSERT_EQ(strcmp((const char *)column1_projectName, projectName.c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column2_bitcodePaths, "tester.bc,testee.bc"), 0);
      ASSERT_EQ(strcmp((const char *)column3_operators, "add_mutation,negate_condition"), 0);
      ASSERT_EQ(strcmp((const char *)column4_dylibs, "sqlite3.dylib,libz.dylib"), 0);
      ASSERT_EQ(strcmp((const char *)column5_tests, "test_method1,test_method2"), 0);
      ASSERT_EQ(column6_fork, true);
      ASSERT_EQ(column7_dryRun, true);
      ASSERT_EQ(column8_useCache, true);
      ASSERT_EQ(column9_timeout, 42);
      ASSERT_EQ(column10_distance, 10);
      ASSERT_EQ(strcmp((const char *)column11_cacheDirectory, "/a/cache"), 0);
      ASSERT_EQ(column12_timeStart, 1234);
      ASSERT_EQ(column13_timeEnd, 5678);

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
