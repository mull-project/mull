#include "Config.h"
#include "Context.h"
#include "SQLiteReporter.h"
#include "Result.h"
#include "MutationOperators/MathAddMutationOperator.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTest_Test.h"
#include "TestModuleFactory.h"
#include "MutationsFinder.h"
#include "Filter.h"
#include "Testee.h"

#include "gtest/gtest.h"

#include <cstring>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <sqlite3.h>

using namespace mull;
using namespace llvm;

TEST(SQLiteReporter, integrationTest) {

  /// STEP 1. Long setup of:
  /// - 1 test with 1 testee with 1 mutation point.
  /// - 1 test execution result which includes 1 normal test execution and 1
  /// mutated test execution.

  TestModuleFactory testModuleFactory;

  auto mullModuleWithTests   = testModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto mullModuleWithTestees = testModuleFactory.create_SimpleTest_CountLetters_Module();

  Context context;
  context.addModule(std::move(mullModuleWithTests));
  context.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::unique_ptr<MathAddMutationOperator> addMutationOperator = make_unique<MathAddMutationOperator>();
  mutationOperators.emplace_back(std::move(addMutationOperator));
  MutationsFinder mutationsFinder(std::move(mutationOperators));
  Filter filter;

  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(context, filter);

  auto &test = *tests.begin();

  Function *testeeFunction = context.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());

  Testee testee(testeeFunction, test.get(), 1);

  std::vector<MutationPoint *> mutationPoints = mutationsFinder.getMutationPoints(context, testee, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = (*(mutationPoints.begin()));

  std::vector<std::string> testIds({
    test->getUniqueIdentifier(),
    test->getUniqueIdentifier()
  });
  std::vector<std::string> mutationPointIds({
    "",
    mutationPoint->getUniqueIdentifier()
  });

  const long long RunningTime_1 = 1;
  const long long RunningTime_2 = 2;

  ExecutionResult testExecutionResult;
  testExecutionResult.status = Passed;
  testExecutionResult.runningTime = RunningTime_1;
  testExecutionResult.stdoutOutput = "testExecutionResult.STDOUT";
  testExecutionResult.stderrOutput = "testExecutionResult.STDERR";

  test->setExecutionResult(testExecutionResult);

  ExecutionResult mutatedTestExecutionResult;
  mutatedTestExecutionResult.status = Failed;
  mutatedTestExecutionResult.runningTime = RunningTime_2;
  mutatedTestExecutionResult.stdoutOutput = "mutatedTestExecutionResult.STDOUT";
  mutatedTestExecutionResult.stderrOutput = "mutatedTestExecutionResult.STDERR";

  auto mutationResult = make_unique<MutationResult>(mutatedTestExecutionResult,
                                                    mutationPoint,
                                                    testee.getDistance(),
                                                    test.get());

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  mutationResults.push_back(std::move(mutationResult));

  ResultTime resultTime(1234, 5678);

  std::unique_ptr<Result> result = make_unique<Result>(std::move(tests),
                                                       std::move(mutationResults),
                                                       mutationPoints);

  /// STEP2. Reporting results to SQLite
  SQLiteReporter reporter("integration test");
  reporter.reportResults(result, Config(), resultTime);

  /// STEP3. Making assertions.
  std::vector<ExecutionResult> executionResults {
    testExecutionResult,
    mutatedTestExecutionResult
  };

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT * FROM execution_result";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, NULL);

  const unsigned char *column_test_id;
  const unsigned char *column_mutation_point_id;
  int column_status;
  int column_duration;
  const unsigned char *column_stdout;
  const unsigned char *column_stderr;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step (selectStmt);

    if (stepResult == SQLITE_ROW) {
      column_test_id = sqlite3_column_text(selectStmt, 0);
      column_mutation_point_id = sqlite3_column_text(selectStmt, 1);

      column_status = sqlite3_column_int(selectStmt, 2);
      column_duration = sqlite3_column_int(selectStmt, 3);

      column_stdout  = sqlite3_column_text(selectStmt, 4);
      column_stderr  = sqlite3_column_text(selectStmt, 5);

      ASSERT_EQ(strcmp((const char *)column_test_id,
                       testIds[numberOfRows].c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column_mutation_point_id,
                       mutationPointIds[numberOfRows].c_str()), 0);

      ASSERT_EQ(column_status, executionResults[numberOfRows].status);
      ASSERT_EQ(column_duration, executionResults[numberOfRows].runningTime);

      ASSERT_EQ(strcmp((const char *)column_stdout,
                       executionResults[numberOfRows].stdoutOutput.c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column_stderr,
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
  std::string projectName("Integration Test Config");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList = "/tmp/dynamic_library_file_list.txt";
  const std::string objectFileList = "/tmp/object_file.list";

  std::ofstream bitcodeFile(bitcodeFileList);
  std::ofstream dynamicLibraryFile(dynamicLibraryFileList);
  std::ofstream objectFile(objectFileList);

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

  if (!objectFile) {
    std::cerr << "Cannot open the output file." << std::endl;

    ASSERT_FALSE(true);
  }

  objectFile << "foo.o" << std::endl;
  objectFile << "bar.o" << std::endl;

  std::vector<std::string> operators({
    "add_mutation",
    "negate_condition"
  });

  std::vector<std::string> selectedTests({
    "test_method1",
    "test_method2"
  });

  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  Config config(bitcodeFileList,
                projectName,
                testFramework,
                operators,
                dynamicLibraryFileList,
                objectFileList,
                selectedTests,
                {}, {},
                Config::Fork::Enabled,
                Config::DryRunMode::Enabled,
                Config::FailFastMode::Enabled,
                Config::UseCache::Yes,
                Config::EmitDebugInfo::No,
                Config::Diagnostics::None,
                timeout, distance,
                cacheDirectory);

  SQLiteReporter reporter(config.getProjectName());

  ResultTime resultTime(1234, 5678);

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  std::vector<std::unique_ptr<mull::Test>> tests;
  std::vector<MutationPoint *> mutationPoints;

  std::unique_ptr<Result> result = make_unique<Result>(std::move(tests),
                                                       std::move(mutationResults),
                                                       mutationPoints);
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
  const unsigned char *column5_objectFiles = nullptr;
  const unsigned char *column6_tests = nullptr;

  int column7_fork = 0;
  int column8_dryRun = 0;
  int column_failFast = 0;
  int column9_useCache = 0;
  int column10_timeout = 0;
  int column11_distance = 0;
  const unsigned char *column12_cacheDirectory = nullptr;
  int column13_timeStart = 0;
  int column14_timeEnd = 0;
  int column15_emitDebugInfo = 0;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step (selectStmt);

    if (stepResult == SQLITE_ROW) {
      int row = 0;
      column1_projectName = sqlite3_column_text(selectStmt, row++);
      column2_bitcodePaths = sqlite3_column_text(selectStmt, row++);
      column3_operators = sqlite3_column_text(selectStmt, row++);
      column4_dylibs = sqlite3_column_text(selectStmt, row++);
      column5_objectFiles = sqlite3_column_text(selectStmt, row++);
      column6_tests = sqlite3_column_text(selectStmt, row++);
      column7_fork = sqlite3_column_int(selectStmt, row++);
      column8_dryRun = sqlite3_column_int(selectStmt, row++);
      column_failFast = sqlite3_column_int(selectStmt, row++);
      column9_useCache = sqlite3_column_int(selectStmt, row++);
      column10_timeout = sqlite3_column_int(selectStmt, row++);
      column11_distance = sqlite3_column_int(selectStmt, row++);
      column12_cacheDirectory = sqlite3_column_text(selectStmt, row++);
      column13_timeStart = sqlite3_column_int(selectStmt, row++);
      column14_timeEnd = sqlite3_column_int(selectStmt, row++);
      column15_emitDebugInfo = sqlite3_column_int(selectStmt, row++);

      ASSERT_EQ(strcmp((const char *)column1_projectName, projectName.c_str()), 0);
      ASSERT_EQ(strcmp((const char *)column2_bitcodePaths, "tester.bc,testee.bc"), 0);
      ASSERT_EQ(strcmp((const char *)column3_operators, "add_mutation,negate_condition"), 0);
      ASSERT_EQ(strcmp((const char *)column4_dylibs, "sqlite3.dylib,libz.dylib"), 0);
      ASSERT_EQ(strcmp((const char *)column5_objectFiles, "foo.o,bar.o"), 0);
      ASSERT_EQ(strcmp((const char *)column6_tests, "test_method1,test_method2"), 0);
      ASSERT_EQ(column7_fork, true);
      ASSERT_EQ(column8_dryRun, true);
      ASSERT_EQ(column_failFast, true);
      ASSERT_EQ(column9_useCache, true);
      ASSERT_EQ(column10_timeout, 42);
      ASSERT_EQ(column11_distance, 10);
      ASSERT_EQ(strcmp((const char *)column12_cacheDirectory, "/a/cache"), 0);
      ASSERT_EQ(column13_timeStart, 1234);
      ASSERT_EQ(column14_timeEnd, 5678);
      ASSERT_EQ(column15_emitDebugInfo, 0);

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

TEST(SQLiteReporter, do_emitDebugInfo) {
  TestModuleFactory testModuleFactory;

  auto mullModuleWithTests   = testModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto mullModuleWithTestees = testModuleFactory.create_SimpleTest_CountLetters_Module();

  Context context;
  context.addModule(std::move(mullModuleWithTests));
  context.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::unique_ptr<MathAddMutationOperator> addMutationOperator = make_unique<MathAddMutationOperator>();
  mutationOperators.emplace_back(std::move(addMutationOperator));
  MutationsFinder mutationsFinder(std::move(mutationOperators));
  Filter filter;
  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(context, filter);

  auto &test = *tests.begin();

  Function *testeeFunction = context.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());

  Testee testee(testeeFunction, test.get(), 1);

  std::vector<MutationPoint *> mutationPoints =
    mutationsFinder.getMutationPoints(context, testee, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = (*(mutationPoints.begin()));

  const long long RunningTime_1 = 1;
  const long long RunningTime_2 = 2;

  ExecutionResult testExecutionResult;
  testExecutionResult.status = Passed;
  testExecutionResult.runningTime = RunningTime_1;
  testExecutionResult.stdoutOutput = "testExecutionResult.STDOUT";
  testExecutionResult.stderrOutput = "testExecutionResult.STDERR";

  ExecutionResult mutatedTestExecutionResult;
  mutatedTestExecutionResult.status = Failed;
  mutatedTestExecutionResult.runningTime = RunningTime_2;
  mutatedTestExecutionResult.stdoutOutput = "mutatedTestExecutionResult.STDOUT";
  mutatedTestExecutionResult.stderrOutput = "mutatedTestExecutionResult.STDERR";

  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  auto mutationResult = make_unique<MutationResult>(mutatedTestExecutionResult,
                                                    mutationPoint,
                                                    testee.getDistance(),
                                                    test.get());
  mutationResults.push_back(std::move(mutationResult));

  ResultTime resultTime(1234, 5678);

  std::unique_ptr<Result> result = make_unique<Result>(std::move(tests),
                                                       std::move(mutationResults),
                                                       mutationPoints);

  std::string projectName("Integration Test Do Emit Debug Info");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList = "/tmp/dynamic_library_file_list.txt";
  const std::string objectFileList = "/tmp/object_file.list";

  std::vector<std::string> operators({
    "add_mutation",
    "negate_condition"
  });

  std::vector<std::string> configTests({
    "test_method1",
    "test_method2"
  });

  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  Config config(bitcodeFileList,
                projectName,
                testFramework,
                operators,
                dynamicLibraryFileList,
                objectFileList,
                configTests,
                {}, {},
                Config::Fork::Enabled,
                Config::DryRunMode::Enabled,
                Config::FailFastMode::Disabled,
                Config::UseCache::Yes,
                Config::EmitDebugInfo::Yes,
                Config::Diagnostics::None,
                timeout, distance,
                cacheDirectory);

  SQLiteReporter reporter(projectName);
  reporter.reportResults(result, config, resultTime);

  std::vector<ExecutionResult> executionResults {
    testExecutionResult,
    mutatedTestExecutionResult
  };

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT count(*) FROM mutation_point_debug";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, NULL);

  int count;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step (selectStmt);

    if (stepResult == SQLITE_ROW) {
      count = sqlite3_column_int(selectStmt, 0);

      ASSERT_EQ(count, 1);

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

TEST(SQLiteReporter, do_not_emitDebugInfo) {
  TestModuleFactory testModuleFactory;

  auto mullModuleWithTests   = testModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto mullModuleWithTestees = testModuleFactory.create_SimpleTest_CountLetters_Module();

  Context context;
  context.addModule(std::move(mullModuleWithTests));
  context.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::unique_ptr<MathAddMutationOperator> addMutationOperator = make_unique<MathAddMutationOperator>();
  mutationOperators.emplace_back(std::move(addMutationOperator));
  MutationsFinder mutationsFinder(std::move(mutationOperators));
  Filter filter;

  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(context, filter);
  auto &test = *tests.begin();

  Function *testeeFunction = context.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());

  Testee testee(testeeFunction, test.get(), 1);

  std::vector<MutationPoint *> mutationPoints =
    mutationsFinder.getMutationPoints(context, testee, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = (*(mutationPoints.begin()));

  const long long RunningTime_1 = 1;
  const long long RunningTime_2 = 2;

  ExecutionResult testExecutionResult;
  testExecutionResult.status = Passed;
  testExecutionResult.runningTime = RunningTime_1;
  testExecutionResult.stdoutOutput = "testExecutionResult.STDOUT";
  testExecutionResult.stderrOutput = "testExecutionResult.STDERR";

  ExecutionResult mutatedTestExecutionResult;
  mutatedTestExecutionResult.status = Failed;
  mutatedTestExecutionResult.runningTime = RunningTime_2;
  mutatedTestExecutionResult.stdoutOutput = "mutatedTestExecutionResult.STDOUT";
  mutatedTestExecutionResult.stderrOutput = "mutatedTestExecutionResult.STDERR";

  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  auto mutationResult = make_unique<MutationResult>(mutatedTestExecutionResult,
                                                    mutationPoint,
                                                    testee.getDistance(),
                                                    test.get());
  mutationResults.push_back(std::move(mutationResult));

  ResultTime resultTime(1234, 5678);

  std::unique_ptr<Result> result = make_unique<Result>(std::move(tests),
                                                       std::move(mutationResults),
                                                       mutationPoints);

  std::string projectName("Integration Test Do Not Emit Debug Info");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList = "/tmp/dynamic_library_file_list.txt";
  const std::string objectFileList = "/tmp/object_file.list";

  std::vector<std::string> operators({
    "add_mutation",
    "negate_condition"
  });

  std::vector<std::string> configTests({
    "test_method1",
    "test_method2"
  });

  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  Config config(bitcodeFileList,
                projectName,
                testFramework,
                operators,
                dynamicLibraryFileList,
                objectFileList,
                configTests,
                {}, {},
                Config::Fork::Enabled,
                Config::DryRunMode::Enabled,
                Config::FailFastMode::Disabled,
                Config::UseCache::Yes,
                Config::EmitDebugInfo::No,
                Config::Diagnostics::None,
                timeout, distance,
                cacheDirectory);

  SQLiteReporter reporter(projectName);
  reporter.reportResults(result, config, resultTime);

  std::vector<ExecutionResult> executionResults {
    testExecutionResult,
    mutatedTestExecutionResult
  };

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT count(*) FROM mutation_point_debug";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(), &selectStmt, NULL);

  int count;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step (selectStmt);

    if (stepResult == SQLITE_ROW) {
      count = sqlite3_column_int(selectStmt, 0);

      ASSERT_EQ(count, 0);

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

