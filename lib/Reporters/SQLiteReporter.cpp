#include "mull/Reporters/SQLiteReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Result.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>

#include <sqlite3.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

using namespace mull;
using namespace llvm;

static void createTables(Diagnostics &diagnostics, sqlite3 *database);

static void sqlite_exec(Diagnostics &diagnostics, sqlite3 *database, const char *sql) {
  char *errorMessage;
  int result = sqlite3_exec(database, sql, nullptr, nullptr, &errorMessage);
  if (result != SQLITE_OK) {
    std::stringstream stringstream;
    stringstream << "Cannot execute " << sql << '\n' << "Reason: '" << errorMessage << "'\n";
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
    return ".";
  }
  return reportDir;
}

SQLiteReporter::SQLiteReporter(Diagnostics &diagnostics, const std::string &reportDir,
                               const std::string &reportName,
                               std::unordered_map<std::string, std::string> mullInformation)
    : diagnostics(diagnostics),
      databasePath(getReportDir(reportDir) + "/" + getReportName(reportName)),
      mullInformation(std::move(mullInformation)) {
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

  const char *insertInformationQuery = "INSERT INTO information VALUES (?1, ?2)";
  sqlite3_stmt *insertInformationStmt;
  sqlite3_prepare(database, insertInformationQuery, -1, &insertInformationStmt, nullptr);

  for (auto &info : mullInformation) {
    sqlite3_bind_text(insertInformationStmt, 1, info.first.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertInformationStmt, 2, info.second.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(insertInformationStmt);
    sqlite3_clear_bindings(insertInformationStmt);
    sqlite3_reset(insertInformationStmt);
  }

  const char *query =
      "INSERT INTO mutant VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12)";
  sqlite3_stmt *stmt;
  sqlite3_prepare(database, query, -1, &stmt, nullptr);

  for (auto &mutationResult : result.getMutationResults()) {
    auto mutant = mutationResult->getMutant();
    auto location = mutant->getSourceLocation();
    auto endLocation = mutant->getEndLocation();

    ExecutionResult execution = mutationResult->getExecutionResult();

    int index = 1;

    sqlite3_bind_text(stmt, index++, mutant->getIdentifier().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, mutant->getMutatorIdentifier().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, location.filePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, location.directory.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, index++, location.line);
    sqlite3_bind_int(stmt, index++, location.column);
    sqlite3_bind_int(stmt, index++, endLocation.line);
    sqlite3_bind_int(stmt, index++, endLocation.column);
    sqlite3_bind_int(stmt, index++, execution.status);
    sqlite3_bind_int64(stmt, index++, execution.runningTime);
    sqlite3_bind_text(stmt, index++, execution.stdoutOutput.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, execution.stderrOutput.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }

  sqlite_exec(diagnostics, database, "END TRANSACTION");

  sqlite3_close(database);

  diagnostics.info(std::string("Results can be found at '") + databasePath + "'");
}

static const char *CreateTables = R"CreateTables(
CREATE TABLE IF NOT EXISTS mutant (
  mutant_id TEXT,
  mutator TEXT,
  filename TEXT,
  directory TEXT,
  line_number INT,
  column_number INT,
  end_line_number INT,
  end_column_number INT,
  status INT,
  duration INT,
  stdout TEXT,
  stderr TEXT
);

CREATE TABLE IF NOT EXISTS information (
  key TEXT,
  value TEXT
);
)CreateTables";

static void createTables(mull::Diagnostics &diagnostics, sqlite3 *database) {
  sqlite_exec(diagnostics, database, CreateTables);
}

RawReport mull::SQLiteReporter::loadRawReport(const std::string &databasePath) {
  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  std::unordered_map<std::string, std::string> information;
  sqlite3_stmt *selectInfoStmt;
  sqlite3_prepare(database, "select * from information", -1, &selectInfoStmt, nullptr);
  while (sqlite3_step(selectInfoStmt) == SQLITE_ROW) {
    auto key = sqlite3_column_text(selectInfoStmt, 0);
    auto value = sqlite3_column_text(selectInfoStmt, 1);
    information[reinterpret_cast<char const *>(key)] = reinterpret_cast<char const *>(value);
  }
  sqlite3_finalize(selectInfoStmt);

  std::unordered_map<std::string, std::vector<ExecutionResult>> mapping;

  sqlite3_stmt *selectMutantsStmt;
  sqlite3_prepare(database, "select * from mutant", -1, &selectMutantsStmt, nullptr);
  while (sqlite3_step(selectMutantsStmt) == SQLITE_ROW) {
    int index = 0;
    std::string mutant_id =
        reinterpret_cast<char const *>(sqlite3_column_text(selectInfoStmt, index++));
    std::string mutator =
        reinterpret_cast<char const *>(sqlite3_column_text(selectInfoStmt, index++));
    std::string filename =
        reinterpret_cast<char const *>(sqlite3_column_text(selectInfoStmt, index++));
    std::string directory =
        reinterpret_cast<char const *>(sqlite3_column_text(selectInfoStmt, index++));
    auto line_number = sqlite3_column_int(selectInfoStmt, index++);
    auto column_number = sqlite3_column_int(selectInfoStmt, index++);
    auto end_line_number = sqlite3_column_int(selectInfoStmt, index++);
    auto end_column_number = sqlite3_column_int(selectInfoStmt, index++);
    auto status = sqlite3_column_int(selectInfoStmt, index++);
    auto duration = sqlite3_column_int64(selectInfoStmt, index++);
    std::string stdout_string =
        reinterpret_cast<char const *>(sqlite3_column_text(selectInfoStmt, index++));
    std::string stderr_string =
        reinterpret_cast<char const *>(sqlite3_column_text(selectInfoStmt, index++));

    ExecutionResult executionResult;
    executionResult.runningTime = duration;
    executionResult.stdoutOutput = stdout_string;
    executionResult.stderrOutput = stderr_string;
    executionResult.status = static_cast<ExecutionStatus>(status);

    SourceLocation location(directory, filename, directory, filename, line_number, column_number);
    SourceLocation endLocation(
        directory, filename, directory, filename, end_line_number, end_column_number);
    mapping[mutant_id].push_back(executionResult);
  }
  sqlite3_finalize(selectMutantsStmt);

  sqlite3_close(database);

  return { .info = std::move(information), .executionResults = std::move(mapping) };
}
