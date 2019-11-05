#include "mull/Reporters/SQLiteReporter.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Config/RawConfig.h"
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
#include <ostream>
#include <sqlite3.h>

using namespace mull;
using namespace llvm;

TEST(SQLiteReporter, integrationTest) {

  /// STEP 1. Long setup of:
  /// - 1 test with 1 testee with 1 mutation point.
  /// - 1 test execution result which includes 1 normal test execution and 1
  /// mutated test execution.
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext);

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
  reachableFunctions.emplace_back(
      make_unique<ReachableFunction>(reachableFunction, nullptr, 1));
  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);
  functionsUnderTest.back().selectInstructions({});
  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(program, functionsUnderTest);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();

  std::vector<std::string> testIds(
      {test.getUniqueIdentifier(), test.getUniqueIdentifier()});
  std::vector<std::string> mutationPointIds(
      {"", mutationPoint->getUniqueIdentifier()});

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
      mutatedTestExecutionResult, mutationPoint,
      reachableFunctions.front()->getDistance(), &test);

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  mutationResults.push_back(std::move(mutationResult));

  MetricsMeasure resultTime;

  Result result(std::move(tests), std::move(mutationResults), mutationPoints);

  /// STEP2. Reporting results to SQLite
  SQLiteReporter reporter("integration test", "");
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, RawConfig(), metrics);

  /// STEP3. Making assertions.
  std::vector<ExecutionResult> executionResults{testExecutionResult,
                                                mutatedTestExecutionResult};

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  {
    std::string selectQuery = "SELECT * FROM execution_result";
    sqlite3_stmt *selectStmt;
    sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(),
                    &selectStmt, nullptr);

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

        ASSERT_EQ(
            strcmp((const char *)column_test_id, testIds[numberOfRows].c_str()),
            0);
        ASSERT_EQ(strcmp((const char *)column_mutation_point_id,
                         mutationPointIds[numberOfRows].c_str()),
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
    sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(),
                    &selectStmt, nullptr);

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
        ASSERT_EQ(strcmp((const char *)test_unique_id, "test_count_letters"),
                  0);

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

TEST(SQLiteReporter, integrationTest_Config) {
  std::string projectName("Integration Test RawConfig");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList =
      "/tmp/dynamic_library_file_list.txt";
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

  std::vector<std::string> operators({"add_mutation", "negate_condition"});

  std::vector<std::string> selectedTests({"test_method1", "test_method2"});

  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  RawConfig config(bitcodeFileList, projectName, testFramework, operators, {},
                   dynamicLibraryFileList, objectFileList, selectedTests, {},
                   {}, RawConfig::Fork::Enabled, RawConfig::DryRunMode::Enabled,
                   RawConfig::FailFastMode::Enabled, RawConfig::UseCache::Yes,
                   RawConfig::EmitDebugInfo::No, Diagnostics::None, timeout,
                   distance, cacheDirectory, JunkDetectionConfig::disabled(),
                   ParallelizationConfig::defaultConfig(), "");

  SQLiteReporter reporter(projectName, "");

  MetricsMeasure resultTime;
  resultTime.begin = MetricsMeasure::Precision(1234);
  resultTime.end = MetricsMeasure::Precision(5678);

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  std::vector<mull::Test> tests;
  std::vector<MutationPoint *> mutationPoints;

  Result result(std::move(tests), std::move(mutationResults), mutationPoints);
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, config, metrics);

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT * FROM config";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(),
                  &selectStmt, nullptr);

  const unsigned char *column1_projectName = nullptr;
  const unsigned char *column2_bitcodePaths = nullptr;
  const unsigned char *column3_mutators = nullptr;
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
    int stepResult = sqlite3_step(selectStmt);

    if (stepResult == SQLITE_ROW) {
      int row = 0;
      column1_projectName = sqlite3_column_text(selectStmt, row++);
      column2_bitcodePaths = sqlite3_column_text(selectStmt, row++);
      column3_mutators = sqlite3_column_text(selectStmt, row++);
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

      ASSERT_EQ(strcmp((const char *)column1_projectName, projectName.c_str()),
                0);
      ASSERT_EQ(
          strcmp((const char *)column2_bitcodePaths, "tester.bc,testee.bc"), 0);
      ASSERT_EQ(strcmp((const char *)column3_mutators,
                       "add_mutation,negate_condition"),
                0);
      ASSERT_EQ(
          strcmp((const char *)column4_dylibs, "sqlite3.dylib,libz.dylib"), 0);
      ASSERT_EQ(strcmp((const char *)column5_objectFiles, "foo.o,bar.o"), 0);
      ASSERT_EQ(
          strcmp((const char *)column6_tests, "test_method1,test_method2"), 0);
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
    } else if (stepResult == SQLITE_DONE) {
      break;
    } else {
      fprintf(stderr, "Failed.\n");
      exit(1);
    }
  }

  ASSERT_EQ(numberOfRows, 1);

  sqlite3_finalize(selectStmt);
  sqlite3_close(database);
}

TEST(SQLiteReporter, do_emitDebugInfo) {
  std::string projectName("Integration Test Do Emit Debug Info");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList =
      "/tmp/dynamic_library_file_list.txt";
  const std::string objectFileList = "/tmp/object_file.list";

  std::vector<std::string> operators({"add_mutation", "negate_condition"});

  std::vector<std::string> configTests({"test_method1", "test_method2"});

  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  RawConfig rawConfig(
      bitcodeFileList, projectName, testFramework, operators, {},
      dynamicLibraryFileList, objectFileList, configTests, {}, {},
      RawConfig::Fork::Enabled, RawConfig::DryRunMode::Enabled,
      RawConfig::FailFastMode::Disabled, RawConfig::UseCache::Yes,
      RawConfig::EmitDebugInfo::Yes, Diagnostics::None, timeout, distance,
      cacheDirectory, JunkDetectionConfig::disabled(),
      ParallelizationConfig::defaultConfig(), "");

  Configuration configuration(rawConfig);

  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  bitcode.push_back(std::move(bitcodeWithTestees));
  Program program({}, {}, std::move(bitcode));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<cxx::AddToSub>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(program);

  auto &test = tests.front();

  Function *reachableFunction = program.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(reachableFunction->empty());

  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  reachableFunctions.emplace_back(
      make_unique<ReachableFunction>(reachableFunction, nullptr, 1));
  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);
  functionsUnderTest.back().selectInstructions({});

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(program, functionsUnderTest);

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

  auto mutationResult = make_unique<MutationResult>(
      mutatedTestExecutionResult, mutationPoint,
      reachableFunctions.front()->getDistance(), &test);
  mutationResults.push_back(std::move(mutationResult));

  MetricsMeasure resultTime;
  resultTime.begin = MetricsMeasure::Precision(1234);
  resultTime.end = MetricsMeasure::Precision(5678);

  Result result(std::move(tests), std::move(mutationResults), mutationPoints);

  SQLiteReporter reporter(projectName, "");
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, rawConfig, metrics);

  std::vector<ExecutionResult> executionResults{testExecutionResult,
                                                mutatedTestExecutionResult};

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT count(*) FROM mutation_point_debug";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(),
                  &selectStmt, nullptr);

  int count;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step(selectStmt);

    if (stepResult == SQLITE_ROW) {
      count = sqlite3_column_int(selectStmt, 0);

      ASSERT_EQ(count, 1);

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
  sqlite3_close(database);
}

TEST(SQLiteReporter, do_not_emitDebugInfo) {
  std::string projectName("Integration Test Do Not Emit Debug Info");
  std::string testFramework = "SimpleTest";

  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList =
      "/tmp/dynamic_library_file_list.txt";
  const std::string objectFileList = "/tmp/object_file.list";

  std::vector<std::string> operators({"add_mutation", "negate_condition"});

  std::vector<std::string> configTests({"test_method1", "test_method2"});

  int timeout = 42;
  int distance = 10;
  std::string cacheDirectory = "/a/cache";
  RawConfig rawConfig(
      bitcodeFileList, projectName, testFramework, operators, {},
      dynamicLibraryFileList, objectFileList, configTests, {}, {},
      RawConfig::Fork::Enabled, RawConfig::DryRunMode::Enabled,
      RawConfig::FailFastMode::Disabled, RawConfig::UseCache::Yes,
      RawConfig::EmitDebugInfo::No, Diagnostics::None, timeout, distance,
      cacheDirectory, JunkDetectionConfig::disabled(),
      ParallelizationConfig::defaultConfig(), "");

  Configuration configuration(rawConfig);

  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);
  auto bitcodeWithTestees = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  bitcode.push_back(std::move(bitcodeWithTestees));
  Program program({}, {}, std::move(bitcode));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<cxx::AddToSub>());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);

  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(program);
  auto &test = *tests.begin();

  Function *reachableFunction = program.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(reachableFunction->empty());

  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  reachableFunctions.emplace_back(
      make_unique<ReachableFunction>(reachableFunction, nullptr, 1));
  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);
  functionsUnderTest.back().selectInstructions({});

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(program, functionsUnderTest);

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

  auto mutationResult = make_unique<MutationResult>(
      mutatedTestExecutionResult, mutationPoint,
      reachableFunctions.front()->getDistance(), &test);
  mutationResults.push_back(std::move(mutationResult));

  MetricsMeasure resultTime;
  resultTime.begin = MetricsMeasure::Precision(1234);
  resultTime.end = MetricsMeasure::Precision(5678);

  Result result(std::move(tests), std::move(mutationResults), mutationPoints);

  SQLiteReporter reporter(projectName, "");
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, rawConfig, metrics);

  std::vector<ExecutionResult> executionResults{testExecutionResult,
                                                mutatedTestExecutionResult};

  std::string databasePath = reporter.getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::string selectQuery = "SELECT count(*) FROM mutation_point_debug";
  sqlite3_stmt *selectStmt;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.size(),
                  &selectStmt, nullptr);

  int count;

  int numberOfRows = 0;
  while (1) {
    int stepResult = sqlite3_step(selectStmt);

    if (stepResult == SQLITE_ROW) {
      count = sqlite3_column_int(selectStmt, 0);

      ASSERT_EQ(count, 0);

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
  sqlite3_close(database);
}
