#include "mull/Reporters/SQLiteReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"
#include "mull/Mutant.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Result.h"
#include "mull/SourceLocation.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>

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

void mull::SQLiteReporter::reportResults(const Result &result) {
  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  createTables(diagnostics, database);

  sqlite_exec(diagnostics, database, "BEGIN TRANSACTION");

  const char *insertExecutionResultQuery =
      "INSERT INTO execution_result VALUES (?1, ?2, ?3, ?4, ?5)";
  sqlite3_stmt *insertExecutionResultStmt;
  sqlite3_prepare(database, insertExecutionResultQuery, -1, &insertExecutionResultStmt, nullptr);

  for (auto &mutationResult : result.getMutationResults()) {
    const std::string &mutantId = mutationResult->getMutant()->getIdentifier();

    ExecutionResult mutationExecutionResult = mutationResult->getExecutionResult();

    int executionResultIndex = 1;
    sqlite3_bind_text(
        insertExecutionResultStmt, executionResultIndex++, mutantId.c_str(), -1, SQLITE_TRANSIENT);
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
  }

  const char *insertMutantQuery = "INSERT INTO mutant VALUES (?1, ?2, ?3, ?4, ?5, ?6)";
  sqlite3_stmt *insertMutantStmt;
  sqlite3_prepare(database, insertMutantQuery, -1, &insertMutantStmt, nullptr);

  for (auto &mutant : result.getMutants()) {
    SourceLocation location = mutant->getSourceLocation();

    int index = 1;
    sqlite3_bind_text(
        insertMutantStmt, index++, mutant->getMutatorIdentifier().c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_bind_text(insertMutantStmt, index++, location.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertMutantStmt, index++, location.directory.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertMutantStmt, index++, location.line);
    sqlite3_bind_int(insertMutantStmt, index++, location.column);

    sqlite3_bind_text(
        insertMutantStmt, index++, mutant->getIdentifier().c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(insertMutantStmt);
    sqlite3_clear_bindings(insertMutantStmt);
    sqlite3_reset(insertMutantStmt);
  }

  sqlite_exec(diagnostics, database, "END TRANSACTION");

  sqlite3_close(database);

  diagnostics.info(std::string("Results can be found at '") + databasePath + "'");
}

#pragma mark - Database Schema

static const char *CreateTables = R"CreateTables(
CREATE TABLE execution_result (
  mutation_point_id TEXT,
  status INT,
  duration INT,
  stdout TEXT,
  stderr TEXT
);

CREATE TABLE mutant (
  mutator TEXT,
  filename TEXT,
  directory TEXT,
  line_number INT,
  column_number INT,
  unique_id TEXT
);
)CreateTables";

static void createTables(mull::Diagnostics &diagnostics, sqlite3 *database) {
  sqlite_exec(diagnostics, database, CreateTables);
}
