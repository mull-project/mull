#include "mull/Reporters/SQLiteReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"
#include "mull/Metrics/Metrics.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Result.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <sqlite3.h>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace mull;
using namespace llvm;

static void createTables(Diagnostics &diagnostics, sqlite3 *database);

void sqlite_exec(Diagnostics &diagnostics, sqlite3 *database, const char *sql) {
  char *errorMessage;
  int result = sqlite3_exec(database, sql, nullptr, nullptr, &errorMessage);
  if (result != SQLITE_OK) {
    std::stringstream stringstream;
    stringstream << "Cannot execute " << sql << '\n' << "Reason: '" << errorMessage << "'\n";
    diagnostics.error(stringstream.str());
  }
}

void sqlite_step(Diagnostics &diagnostics, sqlite3 *database, sqlite3_stmt *stmt) {
  int result = sqlite3_step(stmt);
  if (result != SQLITE_DONE) {
    std::stringstream stringstream;
    stringstream << "Error inserting data: \n"
                 << "Reason: '" << sqlite3_errmsg(database) << "'\n";
    diagnostics.error(stringstream.str());
  }
}

static std::string getReportName(const std::string &name) {
  std::string reportName = name;
  if (reportName.empty()) {
    time_t t;
    time(&t);
    reportName = std::to_string(t);
  }
  return reportName + ".sqlite";
}

static std::string getReportDir(const std::string &reportDir) {
  if (reportDir.empty()) {
    return std::string(".");
  }
  return reportDir;
}

SQLiteReporter::SQLiteReporter(Diagnostics &diagnostics, const std::string &reportDir,
                               const std::string &reportName)
    : diagnostics(diagnostics),
      databasePath(getReportDir(reportDir) + "/" + getReportName(reportName)) {
  llvm::sys::fs::create_directories(reportDir, true);
}

std::string mull::SQLiteReporter::getDatabasePath() {
  return databasePath;
}

void mull::SQLiteReporter::reportResults(const Result &result, const Metrics &metrics) {
  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  createTables(diagnostics, database);

  sqlite_exec(diagnostics, database, "BEGIN TRANSACTION");

  const char *insertExecutionResultQuery =
      "INSERT INTO execution_result VALUES (?1, ?2, ?3, ?4, ?5, ?6)";
  sqlite3_stmt *insertExecutionResultStmt;
  sqlite3_prepare(database, insertExecutionResultQuery, -1, &insertExecutionResultStmt, nullptr);

  const char *insertTestQuery = "INSERT INTO test VALUES (?1, ?2, ?3, ?4)";
  sqlite3_stmt *insertTestStmt;
  sqlite3_prepare(database, insertTestQuery, -1, &insertTestStmt, nullptr);

  for (auto &test : result.getTests()) {
    std::string testName = test.getTestDisplayName();
    std::string testUniqueId = test.getUniqueIdentifier();
    ExecutionResult testExecutionResult = test.getExecutionResult();

    auto testLocation = SourceLocation::locationFromFunction(test.getTestBody());

    int executionResultIndex = 1;
    sqlite3_bind_text(insertExecutionResultStmt,
                      executionResultIndex++,
                      testUniqueId.c_str(),
                      -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt, executionResultIndex++, "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertExecutionResultStmt, executionResultIndex++, testExecutionResult.status);
    sqlite3_bind_int64(
        insertExecutionResultStmt, executionResultIndex++, testExecutionResult.runningTime);
    sqlite3_bind_text(insertExecutionResultStmt,
                      executionResultIndex++,
                      testExecutionResult.stdoutOutput.c_str(),
                      -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt,
                      executionResultIndex++,
                      testExecutionResult.stderrOutput.c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    int testIndex = 1;
    sqlite3_bind_text(insertTestStmt, testIndex++, testName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertTestStmt, testIndex++, testUniqueId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        insertTestStmt, testIndex++, testLocation.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertTestStmt, testIndex++, testLocation.line);

    sqlite_step(diagnostics, database, insertExecutionResultStmt);
    sqlite3_clear_bindings(insertExecutionResultStmt);
    sqlite3_reset(insertExecutionResultStmt);

    sqlite_step(diagnostics, database, insertTestStmt);
    sqlite3_clear_bindings(insertTestStmt);
    sqlite3_reset(insertTestStmt);
  }

  const char *insertMutationResultQuery = "INSERT INTO mutation_result VALUES (?1, ?2, ?3)";
  sqlite3_stmt *insertMutationResultStmt;
  sqlite3_prepare(database, insertMutationResultQuery, -1, &insertMutationResultStmt, nullptr);

  for (auto &mutationResult : result.getMutationResults()) {
    MutationPoint *mutationPoint = mutationResult->getMutationPoint();
    std::string testId = mutationResult->getTest()->getUniqueIdentifier();
    std::string pointId = mutationPoint->getUniqueIdentifier();

    ExecutionResult mutationExecutionResult = mutationResult->getExecutionResult();

    int executionResultIndex = 1;
    sqlite3_bind_text(
        insertExecutionResultStmt, executionResultIndex++, testId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        insertExecutionResultStmt, executionResultIndex++, pointId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(
        insertExecutionResultStmt, executionResultIndex++, mutationExecutionResult.status);
    sqlite3_bind_int64(
        insertExecutionResultStmt, executionResultIndex++, mutationExecutionResult.runningTime);
    sqlite3_bind_text(insertExecutionResultStmt,
                      executionResultIndex++,
                      mutationExecutionResult.stdoutOutput.c_str(),
                      -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertExecutionResultStmt,
                      executionResultIndex++,
                      mutationExecutionResult.stderrOutput.c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    sqlite3_step(insertExecutionResultStmt);
    sqlite3_clear_bindings(insertExecutionResultStmt);
    sqlite3_reset(insertExecutionResultStmt);

    int mutationResultIndex = 1;
    sqlite3_bind_text(
        insertMutationResultStmt, mutationResultIndex++, testId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        insertMutationResultStmt, mutationResultIndex++, pointId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(
        insertMutationResultStmt, mutationResultIndex++, mutationResult->getMutationDistance());

    sqlite3_step(insertMutationResultStmt);
    sqlite3_clear_bindings(insertMutationResultStmt);
    sqlite3_reset(insertMutationResultStmt);
  }

  const char *insertMutationPointQuery =
      "INSERT OR IGNORE INTO mutation_point VALUES (?1, ?2, ?3, ?4, ?5, ?6, "
      "?7, ?8, ?9, ?10, ?11, ?12)";
  sqlite3_stmt *insertMutationPointStmt;
  sqlite3_prepare(database, insertMutationPointQuery, -1, &insertMutationPointStmt, nullptr);

  const char *insertMutationPointDebugQuery =
      "INSERT OR IGNORE INTO mutation_point_debug VALUES (?1, ?2, ?3, ?4, ?5, "
      "?6, ?7, ?8)";
  sqlite3_stmt *insertMutationPointDebugStmt;
  sqlite3_prepare(
      database, insertMutationPointDebugQuery, -1, &insertMutationPointDebugStmt, nullptr);

  for (auto mutationPoint : result.getMutationPoints()) {
    SourceLocation location = mutationPoint->getSourceLocation();

    int index = 1;
    sqlite3_bind_text(insertMutationPointStmt,
                      index++,
                      mutationPoint->getMutator()->getUniqueIdentifier().c_str(),
                      -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationPointStmt,
                      index++,
                      mutationPoint->getBitcode()->getModule()->getModuleIdentifier().c_str(),
                      -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationPointStmt,
                      index++,
                      mutationPoint->getOriginalFunction()->getName().str().c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    sqlite3_bind_int(insertMutationPointStmt, index++, mutationPoint->getAddress().getFnIndex());
    sqlite3_bind_int(insertMutationPointStmt, index++, mutationPoint->getAddress().getBBIndex());
    sqlite3_bind_int(insertMutationPointStmt, index++, mutationPoint->getAddress().getIIndex());

    sqlite3_bind_text(
        insertMutationPointStmt, index++, location.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        insertMutationPointStmt, index++, location.directory.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutationPointStmt,
                      index++,
                      mutationPoint->getDiagnostics().c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    sqlite3_bind_int(insertMutationPointStmt, index++, location.line);
    sqlite3_bind_int(insertMutationPointStmt, index++, location.column);

    sqlite3_bind_text(insertMutationPointStmt,
                      index++,
                      mutationPoint->getUniqueIdentifier().c_str(),
                      -1,
                      SQLITE_TRANSIENT);

    sqlite3_step(insertMutationPointStmt);
    sqlite3_clear_bindings(insertMutationPointStmt);
    sqlite3_reset(insertMutationPointStmt);
  }

  sqlite_exec(diagnostics, database, "END TRANSACTION");

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
  ideDiagnostics TEXT,
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

static void createTables(mull::Diagnostics &diagnostics, sqlite3 *database) {
  sqlite_exec(diagnostics, database, CreateTables);
}
