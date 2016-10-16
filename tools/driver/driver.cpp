#include "Driver.h"
#include "ConfigParser.h"
#include "Config.h"
#include "ModuleLoader.h"
#include "MutationPoint.h"

#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"

#include "GoogleTest/GoogleTestFinder.h"
#include "GoogleTest/GoogleTestRunner.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/TargetSelect.h"

#include <ctime>
#include <string>

#include <sqlite3.h>
#include <sys/param.h>
#include <unistd.h>

using namespace Mutang;
using namespace llvm;

int debug_main(int argc, char *argv[]) {
  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(argv[argc - 1]);

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);

  GoogleTestFinder TestFinder;
  GoogleTestRunner Runner;

  Driver D(*Cfg.get(), Loader, TestFinder, Runner);

  if (strcmp(argv[2], "-print-test-names") == 0) {
    D.debug_PrintTestNames();
  } else if (strcmp(argv[2], "-print-testee-names") == 0) {
    D.debug_PrintTesteeNames();
  } else if (strcmp(argv[2], "-print-mutation-points") == 0) {
    D.debug_PrintMutationPoints();
  }

  return 0;
}

void reportResults(const std::vector<std::unique_ptr<TestResult>> &results);

int main(int argc, char *argv[]) {
  if (strcmp(argv[1], "-debug") == 0) {
    return debug_main(argc, argv);
  }

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(argv[1]);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);

#if 1
  GoogleTestFinder TestFinder;
  GoogleTestRunner Runner;
#else
  SimpleTestFinder TestFinder;
  SimpleTestRunner Runner;
#endif

  Driver driver(*Cfg.get(), Loader, TestFinder, Runner);
  auto results = driver.Run();
  reportResults(results);
  llvm_shutdown();
  return 0;
}

std::string getDatabasePath() {
  char wd[MAXPATHLEN] = { 0 };
  getwd(wd);
  std::string currentDirectory(wd);

  time_t ct;
  time(&ct);
  std::string currentTime = std::to_string(ct);

  std::string databasePath = currentDirectory + "/" + currentTime + ".sqlite";
  return databasePath;
}

void sqlite_exec(sqlite3 *database, const char *sql) {
  char *errorMessage;
  int result = sqlite3_exec(database,
                            sql,
                            nullptr,
                            nullptr,
                            &errorMessage);
  if (result != SQLITE_OK) {
    printf("Cannot execute '%s'\n", sql);
    printf("Reason: '%s'\n", errorMessage);
    printf("Shutting down\n");
    exit(18);
  }
}

void createTables(sqlite3 *database) {
  const char *executionResult = "CREATE TABLE execution_result (status INT, duration INT);";
  const char *test = "CREATE TABLE test (test_name TEXT, execution_result_id INT);";
  const char *mutationPoint = "CREATE TABLE mutation_point (mutation_operator TEXT, module_name TEXT, function_index INT, basic_block_index INT, instruction_index INT, filename TEXT, line_number INT);";
  const char *mutationResult = "CREATE TABLE mutation_result (execution_result_id INT, test_id INT, mutation_point_id INT);";

  sqlite_exec(database, executionResult);
  sqlite_exec(database, test);
  sqlite_exec(database, mutationPoint);
  sqlite_exec(database, mutationResult);
}

void reportResults(const std::vector<std::unique_ptr<TestResult>> &results) {
  std::string databasePath = getDatabasePath();
  outs() << "Results can be found at '" << databasePath << "'\n";

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  createTables(database);

  for (auto &result : results) {
    ExecutionResult testResult = result->getOriginalTestResult();
    std::string insertResultSQL = std::string("INSERT INTO execution_result VALUES (")
      + "'" + std::to_string(testResult.Status) + "',"
      + "'" + std::to_string(testResult.RunningTime) + "');";
    sqlite_exec(database, insertResultSQL.c_str());
    int testResultID = sqlite3_last_insert_rowid(database);

    std::string insertTestSQL = std::string("INSERT INTO test VALUES (")
      + "'" + result->getTestName() + "',"
      + "'" + std::to_string(testResultID) + "');";
    sqlite_exec(database, insertTestSQL.c_str());
    int testID = sqlite3_last_insert_rowid(database);

    for (auto &mutation : result->getMutationResults()) {
      auto mutationPoint = mutation->getMutationPoint();
      Instruction *instruction = dyn_cast<Instruction>(mutationPoint->getOriginalValue());
      std::string insertMutationPointSQL = std::string("INSERT INTO mutation_point VALUES (")
        + "'add_mutation_operator',"
        + "'" + instruction->getParent()->getParent()->getParent()->getModuleIdentifier() + "',"
        + "'" + std::to_string(mutationPoint->getAddress().getFnIndex()) + "',"
        + "'" + std::to_string(mutationPoint->getAddress().getBBIndex()) + "',"
        + "'" + std::to_string(mutationPoint->getAddress().getIIndex()) + "',"
        + "'" + instruction->getDebugLoc()->getFilename().str() + "',"
        + "'" + std::to_string(instruction->getDebugLoc()->getLine()) + "');";

      sqlite_exec(database, insertMutationPointSQL.c_str());
      int mutationPointID = sqlite3_last_insert_rowid(database);

      ExecutionResult mutationExecutionResult = mutation->getExecutionResult();
      std::string insertMutationExecutionResultSQL = std::string("INSERT INTO execution_result VALUES (")
        + "'" + std::to_string(mutationExecutionResult.Status) + "',"
        + "'" + std::to_string(mutationExecutionResult.RunningTime) + "');";
      sqlite_exec(database, insertMutationExecutionResultSQL.c_str());

      int mutationExecutionResultID = sqlite3_last_insert_rowid(database);

      std::string insertMutationResultSQL = std::string("INSERT INTO mutation_result VALUES (")
        + "'" + std::to_string(mutationExecutionResultID) + "',"
        + "'" + std::to_string(testID) + "',"
        + "'" + std::to_string(mutationPointID) + "');";

      sqlite_exec(database, insertMutationResultSQL.c_str());
    }

  }

  sqlite3_close(database);
}
