#include "mull/Reporters/SQLiteReporter.h"

#include "mull/Config/RawConfig.h"
#include "mull/ExecutionResult.h"
#include "mull/Logger.h"
#include "mull/MullModule.h"
#include "mull/MutationResult.h"
#include "mull/Result.h"

#include "mull/Metrics/Metrics.h"
#include "mull/Mutators/Mutator.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <sqlite3.h>
#include <sstream>
#include <string>
#include <sys/param.h>
#include <unistd.h>

using namespace mull;
using namespace llvm;

static void createTables(sqlite3 *database);

static std::string vectorToCsv(const std::vector<std::string> &v) {
  if (v.empty()) {
    return std::string();
  }

  std::stringstream csv;
  std::copy(v.begin(), v.end() - 1,
            std::ostream_iterator<std::string>(csv, ","));
  csv << *(v.end() - 1);
  return csv.str();
}

void sqlite_exec(sqlite3 *database, const char *sql) {
  char *errorMessage;
  int result = sqlite3_exec(database, sql, nullptr, nullptr, &errorMessage);
  if (result != SQLITE_OK) {
    Logger::error() << "Cannot execute " << sql << '\n';
    Logger::error() << "Reason: '" << errorMessage << "'\n";
    Logger::error() << "Shutting down\n";
    exit(18);
  }
}

void sqlite_step(sqlite3 *database, sqlite3_stmt *stmt) {
  int result = sqlite3_step(stmt);
  if (result != SQLITE_DONE) {
    Logger::error() << "Error inserting data: \n"
                    << "\n";
    Logger::error() << "Reason: '" << sqlite3_errmsg(database) << "'\n";
    Logger::error() << "Shutting down\n";
    exit(18);
  }
}

SQLiteReporter::SQLiteReporter(const std::string &projectName) {
  SmallString<MAXPATHLEN> databasePath;
  auto error = llvm::sys::fs::current_path(databasePath);
  if (error) {
    Logger::error() << error.message() << "\n";
  }

  time_t ct;
  time(&ct);
  std::string currentTime = std::to_string(ct);
  std::string projectNameComponent = projectName;
  if (!projectNameComponent.empty()) {
    projectNameComponent += "_";
  }

  llvm::sys::path::append(databasePath,
                          projectNameComponent + currentTime + ".sqlite");

  this->databasePath = databasePath.str();
}

std::string mull::SQLiteReporter::getDatabasePath() { return databasePath; }

void mull::SQLiteReporter::reportResults(const Result &result,
                                         const RawConfig &config,
                                         const Metrics &metrics) {

  std::string databasePath = getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  createTables(database);

  sqlite_exec(database, "BEGIN TRANSACTION");

  const char *insertExecutionResultQuery =
      "INSERT INTO execution_result VALUES (?1, ?2, ?3, ?4, ?5, ?6)";
  sqlite3_stmt *insertExecutionResultStmt;
  sqlite3_prepare(database, insertExecutionResultQuery, -1,
                  &insertExecutionResultStmt, NULL);

  const char *insertTestQuery = "INSERT INTO test VALUES (?1, ?2, ?3, ?4)";
  sqlite3_stmt *insertTestStmt;
  sqlite3_prepare(database, insertTestQuery, -1, &insertTestStmt, NULL);

  for (auto &test : result.getTests()) {
    std::string testName = test->getTestDisplayName();
    std::string testUniqueId = test->getUniqueIdentifier();
    ExecutionResult testExecutionResult = test->getExecutionResult();

    auto testLocation =
        SourceLocation::sourceLocationFromFunction(test->testBodyFunction());

    int executionResultIndex = 1;
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      testUniqueId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++, "", -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(insertExecutionResultStmt, executionResultIndex++,
                     testExecutionResult.status);
    sqlite3_bind_int64(insertExecutionResultStmt, executionResultIndex++,
                       testExecutionResult.runningTime);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      testExecutionResult.stdoutOutput.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      testExecutionResult.stderrOutput.c_str(), -1,
                      SQLITE_TRANSIENT);

    int testIndex = 1;
    sqlite3_bind_text(insertTestStmt, testIndex++, testName.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertTestStmt, testIndex++, testUniqueId.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertTestStmt, testIndex++,
                      testLocation.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertTestStmt, testIndex++, testLocation.line);

    sqlite_step(database, insertExecutionResultStmt);
    sqlite3_clear_bindings(insertExecutionResultStmt);
    sqlite3_reset(insertExecutionResultStmt);

    sqlite_step(database, insertTestStmt);
    sqlite3_clear_bindings(insertTestStmt);
    sqlite3_reset(insertTestStmt);
  }

  const char *insertMutationResultQuery =
      "INSERT INTO mutation_result VALUES (?1, ?2, ?3)";
  sqlite3_stmt *insertMutationResultStmt;
  sqlite3_prepare(database, insertMutationResultQuery, -1,
                  &insertMutationResultStmt, nullptr);

  for (auto &mutationResult : result.getMutationResults()) {
    MutationPoint *mutationPoint = mutationResult->getMutationPoint();
    std::string testId = mutationResult->getTest()->getUniqueIdentifier();
    std::string pointId = mutationPoint->getUniqueIdentifier();

    ExecutionResult mutationExecutionResult =
        mutationResult->getExecutionResult();

    int executionResultIndex = 1;
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      testId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      pointId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertExecutionResultStmt, executionResultIndex++,
                     mutationExecutionResult.status);
    sqlite3_bind_int64(insertExecutionResultStmt, executionResultIndex++,
                       mutationExecutionResult.runningTime);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      mutationExecutionResult.stdoutOutput.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++,
                      mutationExecutionResult.stderrOutput.c_str(), -1,
                      SQLITE_TRANSIENT);

    sqlite3_step(insertExecutionResultStmt);
    sqlite3_clear_bindings(insertExecutionResultStmt);
    sqlite3_reset(insertExecutionResultStmt);

    int mutationResultIndex = 1;
    sqlite3_bind_text(insertMutationResultStmt, mutationResultIndex++,
                      testId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationResultStmt, mutationResultIndex++,
                      pointId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertMutationResultStmt, mutationResultIndex++,
                     mutationResult->getMutationDistance());

    sqlite3_step(insertMutationResultStmt);
    sqlite3_clear_bindings(insertMutationResultStmt);
    sqlite3_reset(insertMutationResultStmt);
  }

  const char *insertMutationPointQuery =
      "INSERT OR IGNORE INTO mutation_point VALUES (?1, ?2, ?3, ?4, ?5, ?6, "
      "?7, ?8, ?9, ?10, ?11, ?12)";
  sqlite3_stmt *insertMutationPointStmt;
  sqlite3_prepare(database, insertMutationPointQuery, -1,
                  &insertMutationPointStmt, NULL);

  const char *insertMutationPointDebugQuery =
      "INSERT OR IGNORE INTO mutation_point_debug VALUES (?1, ?2, ?3, ?4, ?5, "
      "?6, ?7, ?8)";
  sqlite3_stmt *insertMutationPointDebugStmt;
  sqlite3_prepare(database, insertMutationPointDebugQuery, -1,
                  &insertMutationPointDebugStmt, NULL);

  for (auto mutationPoint : result.getMutationPoints()) {
    Instruction *instruction =
        dyn_cast<Instruction>(mutationPoint->getOriginalValue());

    SourceLocation location =
        mutationPoint
            ->getSourceLocation(); // SourceLocation::sourceLocationFromInstruction(instruction);

    int index = 1;
    sqlite3_bind_text(
        insertMutationPointStmt, index++,
        mutationPoint->getMutator()->getUniqueIdentifier().c_str(), -1,
        SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationPointStmt, index++,
                      mutationPoint->getOriginalModule()
                          ->getModule()
                          ->getModuleIdentifier()
                          .c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        insertMutationPointStmt, index++,
        mutationPoint->getOriginalFunction()->getName().str().c_str(), -1,
        SQLITE_TRANSIENT);

    sqlite3_bind_int(insertMutationPointStmt, index++,
                     mutationPoint->getAddress().getFnIndex());
    sqlite3_bind_int(insertMutationPointStmt, index++,
                     mutationPoint->getAddress().getBBIndex());
    sqlite3_bind_int(insertMutationPointStmt, index++,
                     mutationPoint->getAddress().getIIndex());

    sqlite3_bind_text(insertMutationPointStmt, index++,
                      location.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationPointStmt, index++,
                      location.directory.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationPointStmt, index++,
                      mutationPoint->getDiagnostics().c_str(), -1,
                      SQLITE_TRANSIENT);

    sqlite3_bind_int(insertMutationPointStmt, index++, location.line);
    sqlite3_bind_int(insertMutationPointStmt, index++, location.column);

    sqlite3_bind_text(insertMutationPointStmt, index++,
                      mutationPoint->getUniqueIdentifier().c_str(), -1,
                      SQLITE_TRANSIENT);

    sqlite3_step(insertMutationPointStmt);
    sqlite3_clear_bindings(insertMutationPointStmt);
    sqlite3_reset(insertMutationPointStmt);

    std::string mutationPointID = mutationPoint->getUniqueIdentifier();

    if (config.shouldEmitDebugInfo()) {
      std::string function;
      llvm::raw_string_ostream f_ostream(function);
      instruction->getFunction()->print(f_ostream);

      std::string basicBlock;
      llvm::raw_string_ostream bb_ostream(basicBlock);
      instruction->getParent()->print(bb_ostream);

      std::string instr;
      llvm::raw_string_ostream i_ostream(instr);
      instruction->print(i_ostream);

      int index = 1;
      sqlite3_bind_text(insertMutationPointDebugStmt, index++,
                        location.filePath.c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(insertMutationPointDebugStmt, index++,
                        location.directory.c_str(), -1, SQLITE_TRANSIENT);

      sqlite3_bind_int(insertMutationPointDebugStmt, index++, location.line);
      sqlite3_bind_int(insertMutationPointDebugStmt, index++, location.column);

      sqlite3_bind_text(insertMutationPointDebugStmt, index++,
                        f_ostream.str().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(insertMutationPointDebugStmt, index++,
                        bb_ostream.str().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(insertMutationPointDebugStmt, index++,
                        i_ostream.str().c_str(), -1, SQLITE_TRANSIENT);

      sqlite3_bind_text(insertMutationPointDebugStmt, index++,
                        mutationPoint->getUniqueIdentifier().c_str(), -1,
                        SQLITE_TRANSIENT);

      sqlite3_step(insertMutationPointDebugStmt);
      sqlite3_clear_bindings(insertMutationPointDebugStmt);
      sqlite3_reset(insertMutationPointDebugStmt);
    }
  }

  /// Config
  {
    const char *insertConfigQuery =
        "INSERT INTO config VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, "
        "?11, ?12, ?13, ?14, ?15)";
    sqlite3_stmt *insertConfigStmt;
    sqlite3_prepare(database, insertConfigQuery, -1, &insertConfigStmt, NULL);

    // Start and end times are not part of a config however we are
    // mixing them in to make them into a final report.
    const long startTime = metrics.driverRunTime().begin.count();
    const long endTime = metrics.driverRunTime().end.count();

    std::string csvBitcodePaths = vectorToCsv(config.getBitcodePaths());
    std::string csvMutators = vectorToCsv(config.getMutators());
    std::string csvDynamicLibraries =
        vectorToCsv(config.getDynamicLibrariesPaths());
    std::string csvObjectFiles = vectorToCsv(config.getObjectFilesPaths());
    std::string csvTests = vectorToCsv(config.getTests());

    int index = 1;
    sqlite3_bind_text(insertConfigStmt, index++,
                      config.getProjectName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertConfigStmt, index++, csvBitcodePaths.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertConfigStmt, index++, csvMutators.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertConfigStmt, index++, csvDynamicLibraries.c_str(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertConfigStmt, index++, csvObjectFiles.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertConfigStmt, index++, csvTests.c_str(), -1,
                      SQLITE_TRANSIENT);

    sqlite3_bind_int(insertConfigStmt, index++, config.forkEnabled());
    sqlite3_bind_int(insertConfigStmt, index++, config.dryRunModeEnabled());
    sqlite3_bind_int(insertConfigStmt, index++, config.failFastModeEnabled());
    sqlite3_bind_int(insertConfigStmt, index++, config.cachingEnabled());
    sqlite3_bind_int(insertConfigStmt, index++, config.getTimeout());
    sqlite3_bind_int(insertConfigStmt, index++, config.getMaxDistance());

    sqlite3_bind_text(insertConfigStmt, index++,
                      config.getCacheDirectory().c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_int64(insertConfigStmt, index++, startTime);
    sqlite3_bind_int64(insertConfigStmt, index++, endTime);

    sqlite3_step(insertConfigStmt);
  }

  sqlite_exec(database, "END TRANSACTION");

  sqlite3_close(database);

  outs() << "Results can be found at '" << databasePath << "'\n";
}

#pragma mark - Database Schema

static const char *CreateTables = R"CreateTables(
CREATE TABLE execution_result (
  test_id TEXT,
  mutation_point_id TEXT,
  status INT,
  duration INT,
  stdout TEXT,
  stderr TEXT
);

CREATE TABLE test (
  test_name TEXT,
  unique_id TEXT UNIQUE,
  location_file TEXT,
  location_line INT
);

CREATE TABLE mutation_point (
  mutator TEXT,
  module_name TEXT,
  function_name TEXT,
  function_index INT,
  basic_block_index INT,
  instruction_index INT,
  filename TEXT,
  directory TEXT,
  diagnostics TEXT,
  line_number INT,
  column_number INT,
  unique_id TEXT UNIQUE
);

CREATE TABLE mutation_result (
  test_id TEXT,
  mutation_point_id TEXT,
  mutation_distance INT
);

CREATE TABLE mutation_point_debug (
  filename TEXT,
  directory TEXT,
  line_number INT,
  column_number INT,
  function TEXT,
  basic_block TEXT,
  instruction TEXT,
  unique_id TEXT UNIQUE
);

CREATE TABLE config (
  project_name TEXT,
  bitcode_paths TEXT,
  mutators TEXT,
  dynamic_libraries TEXT,
  object_files TEXT,
  tests TEXT,
  fork INT,
  dry_run INT,
  fail_fast INT,
  use_cache INT,
  timeout INT,
  max_distance INT,
  cache_directory TEXT,
  time_start INT,
  time_end INT
);
)CreateTables";

static void createTables(sqlite3 *database) {
  sqlite_exec(database, CreateTables);
}
