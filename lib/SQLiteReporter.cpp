#include "SQLiteReporter.h"

#include "Config.h"
#include "Logger.h"
#include "Result.h"
#include "TestResult.h"

#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include <sqlite3.h>
#include <sstream>
#include <string>
#include <sys/param.h>
#include <unistd.h>

using namespace mull;
using namespace llvm;

static void createTables(sqlite3 *database);

static void assume(bool condition, const char *assumption) {
  if (condition) {
    return;
  }

  Logger::warn() << "Assumption failed: " << assumption << '\n';
  exit(1);
}

static std::string vectorToCsv(const std::vector<std::string> &v) {
  if (v.empty()) {
    return std::string();
  }

  std::stringstream csv;
  std::copy(v.begin(), v.end() - 1, std::ostream_iterator<std::string>(csv, ","));
  csv << *(v.end() - 1);
  return csv.str();
}

void sqlite_exec(sqlite3 *database, const char *sql) {
  char *errorMessage;
  int result = sqlite3_exec(database,
                            sql,
                            nullptr,
                            nullptr,
                            &errorMessage);
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
    Logger::error() << "Error inserting data: \n" << "\n";
    Logger::error() << "Reason: '" << sqlite3_errmsg(database) << "'\n";
    Logger::error() << "Shutting down\n";
    exit(18);
  }
}

SQLiteReporter::SQLiteReporter(const std::string &projectName) {
  char wd[MAXPATHLEN] = { 0 };
  getwd(wd);
  std::string currentDirectory(wd);

  time_t ct;
  time(&ct);
  std::string currentTime = std::to_string(ct);
  std::string projectNameComponent = projectName;
  if (!projectNameComponent.empty()) {
    projectNameComponent += "_";
  }

  std::string databasePath = currentDirectory + "/" + projectNameComponent + currentTime + ".sqlite";

  this->databasePath = databasePath;
}

std::string mull::SQLiteReporter::getDatabasePath() {
  return databasePath;
}

void mull::SQLiteReporter::reportResults(const std::unique_ptr<Result> &result,
                                         const Config &config,
                                         const ResultTime &resultTime) {

  std::string databasePath = getDatabasePath();

  sqlite3 *database;
  sqlite3_open(databasePath.c_str(), &database);

  createTables(database);

  for (auto &testResult : result->getTestResults()) {
    std::string testID = testResult->getDisplayName();

    ExecutionResult testExecutionResult = testResult->getOriginalTestResult();
    std::string insertResultSQL = std::string("INSERT INTO execution_result VALUES (")
      + "'" + std::to_string(testExecutionResult.Status) + "',"
      + "'" + std::to_string(testExecutionResult.RunningTime) + "',"
      + "" + "?1" + ","
      + "" + "?2" + ");";
    sqlite3_stmt *stmt;
    sqlite3_prepare(database, insertResultSQL.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, testExecutionResult.stdoutOutput.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, testExecutionResult.stderrOutput.c_str(), -1, SQLITE_STATIC);
    
    sqlite_step(database, stmt);
    
    int testResultID = sqlite3_last_insert_rowid(database);

    std::string insertTestSQL = std::string("INSERT INTO test VALUES (")
      + "'" + testID + "',"
      + "'" + std::to_string(testResultID) + "');";
    sqlite_exec(database, insertTestSQL.c_str());

    for (auto &mutation : testResult->getMutationResults()) {

      /// Mutation Point
      auto mutationPoint = mutation->getMutationPoint();
      Instruction *instruction = dyn_cast<Instruction>(mutationPoint->getOriginalValue());

      std::string fileNameOrNil = "no-debug-info";
      std::string lineOrNil = "0";
      std::string columnOrNil = "0";

      if (instruction->getMetadata(0)) {
        fileNameOrNil = instruction->getDebugLoc()->getFilename().str();
        lineOrNil = std::to_string(instruction->getDebugLoc()->getLine());
        columnOrNil = std::to_string(instruction->getDebugLoc()->getColumn());
      }

      std::string insertMutationPointSQL = std::string("INSERT OR IGNORE INTO mutation_point VALUES (")
        + "'" + mutationPoint->getOperator()->uniqueID() + "',"
        + "'" + instruction->getParent()->getParent()->getParent()->getModuleIdentifier() + "',"
        + "'" + instruction->getParent()->getParent()->getName().str() + "',"
        + "'" + std::to_string(mutationPoint->getAddress().getFnIndex()) + "',"
        + "'" + std::to_string(mutationPoint->getAddress().getBBIndex()) + "',"
        + "'" + std::to_string(mutationPoint->getAddress().getIIndex()) + "',"
        + "'" + fileNameOrNil + "',"
        + "'" + lineOrNil + "',"
        + "'" + columnOrNil + "',"
        + "'" + mutationPoint->getUniqueIdentifier() + "'"+
        + ");";

      sqlite_exec(database, insertMutationPointSQL.c_str());
      std::string mutationPointID = mutationPoint->getUniqueIdentifier();

      {
        std::string function;
        llvm::raw_string_ostream f_ostream(function);
        instruction->getFunction()->print(f_ostream);

        std::string basicBlock;
        llvm::raw_string_ostream bb_ostream(basicBlock);
        instruction->getParent()->print(bb_ostream);

        std::string instr;
        llvm::raw_string_ostream i_ostream(instr);
        instruction->print(i_ostream);

        std::string fileNameOrNil = "no-debug-info";
        std::string lineOrNil = "0";
        std::string columnOrNil = "0";

        if (instruction->getMetadata(0)) {
          fileNameOrNil = instruction->getDebugLoc()->getFilename().str();
          lineOrNil = std::to_string(instruction->getDebugLoc()->getLine());
          columnOrNil = std::to_string(instruction->getDebugLoc()->getColumn());
        }

        std::string insertMutationPointDebugSQL = std::string("INSERT OR IGNORE INTO mutation_point_debug VALUES (")
        + "'" + fileNameOrNil + "',"
        + "'" + lineOrNil + "',"
        + "'" + columnOrNil + "',"
        + "'" + f_ostream.str() + "',"
        + "'" + bb_ostream.str() + "',"
        + "'" + i_ostream.str() + "',"
        + "'" + mutationPoint->getUniqueIdentifier() + "'"+
        + ");";

        sqlite3_stmt *statement = NULL;
        int statement_prepare_result =
        sqlite3_prepare_v2(database,
                           insertMutationPointDebugSQL.c_str(),
                           insertMutationPointDebugSQL.size(),
                           &statement,
                           NULL);

        assume(statement_prepare_result == SQLITE_OK, "SQLite error: Expected finalize of debug mutation point statement to succeed.");

        int insertion_result = sqlite3_step(statement);
        assume(insertion_result == SQLITE_DONE, "SQLite error: Expected insertion of debug mutation point to succeed.");

        int finalize_result = sqlite3_finalize(statement);
        assume(finalize_result == SQLITE_OK, "SQLite error: Expected finalize of debug mutation point statement to succeed.");
      }

      /// Execution result
      ExecutionResult mutationExecutionResult = mutation->getExecutionResult();
      std::string insertMutationExecutionResultSQL = std::string("INSERT INTO execution_result VALUES (")
        + "'" + std::to_string(mutationExecutionResult.Status) + "',"
        + "'" + std::to_string(mutationExecutionResult.RunningTime) + "',"
        + "" + "?" + ","
        + "" + "?" + ");";

      sqlite3_stmt *execution_result_statement = NULL;
      int execution_result_statement_prepare_result =
        sqlite3_prepare_v2(database,
                           insertMutationExecutionResultSQL.c_str(),
                           insertMutationExecutionResultSQL.size(),
                           &execution_result_statement,
                           NULL);

      assume(execution_result_statement_prepare_result == SQLITE_OK,
             "SQLite error: Expected preparation of execution result statement to succeed.");

      if (execution_result_statement_prepare_result != SQLITE_OK) {
        Logger::error() << insertMutationExecutionResultSQL << '\n';
        Logger::error() << sqlite3_errmsg(database) << '\n';
      }

      sqlite3_bind_text(execution_result_statement, 1, mutationExecutionResult.stdoutOutput.c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(execution_result_statement, 2, mutationExecutionResult.stderrOutput.c_str(), -1, SQLITE_TRANSIENT);

      int execution_result_insertion_result = sqlite3_step(execution_result_statement);
      assume(execution_result_insertion_result == SQLITE_DONE,
             "SQLite error: Expected insertion of execution result to succeed.");

      int execution_result_statement_finalize_result
        = sqlite3_finalize(execution_result_statement);
      assume(execution_result_statement_finalize_result == SQLITE_OK,
             "SQLite error: Expected finalize of execution result statement to succeed.");

      std::vector<std::string> callerPath = result.get()->calculateCallerPath(mutation.get());

      std::string callerPathAsString = getCallerPathAsString(callerPath);

      int mutationExecutionResultID = sqlite3_last_insert_rowid(database);
      std::string insertMutationResultSQL = std::string("INSERT INTO mutation_result VALUES (")
          + "'" + std::to_string(mutationExecutionResultID) + "',"
          + "'" + testID + "',"
          + "'" + mutationPointID + "',"
          + "'" + std::to_string(mutation->getMutationDistance()) + "',"
          + "'" + callerPathAsString + "'"
          ");";

      sqlite_exec(database, insertMutationResultSQL.c_str());
    }
  }

  /// Config
  {
    // Start and end times are not part of a config however we are
    // mixing them in to make them into a final report.
    const long startTime = resultTime.start;
    const long endTime = resultTime.end;

    std::string csvBitcodePaths = vectorToCsv(config.getBitcodePaths());
    std::string csvMutationOperators = vectorToCsv(config.getMutationOperators());
    std::string csvDynamicLibraries = vectorToCsv(config.getDynamicLibrariesPaths());
    std::string csvTests = vectorToCsv(config.getTests());

    std::string insertConfigSQL = std::string("INSERT INTO config VALUES (")
    + "'" + config.getProjectName() + "',"
    + "'" + csvBitcodePaths + "',"
    + "'" + csvMutationOperators + "',"
    + "'" + csvDynamicLibraries + "',"
    + "'" + csvTests + "',"
    + "'" + std::to_string(config.getFork()) + "',"
    + "'" + std::to_string(config.isDryRun()) + "',"
    + "'" + std::to_string(config.getUseCache()) + "',"
    + "'" + std::to_string(config.getTimeout()) + "',"
    + "'" + std::to_string(config.getMaxDistance()) + "',"
    + "'" + config.getCacheDirectory() + "'," +
    + "'" + std::to_string(startTime) + "'," +
    + "'" + std::to_string(endTime) + "'" +
    ");"
    ;

    sqlite_exec(database, insertConfigSQL.c_str());
  }

  sqlite3_close(database);

  outs() << "Results can be found at '" << databasePath << "'\n";
}

#pragma mark -

std::string SQLiteReporter::getCallerPathAsString(const std::vector<std::string> &callerPath) {
  std::string callerPathAsString;

  unsigned int indentation = 0;
  for (size_t i = 0; i < callerPath.size(); ++i) {
    if (i != 0) {
      callerPathAsString.append("\n");
    }

    callerPathAsString.append(std::string(2 * indentation++, ' '));
    callerPathAsString.append(callerPath[i]);
  }

  return callerPathAsString;
}

#pragma mark - Database Schema

static const char *CreateTables = R"CreateTables(
CREATE TABLE execution_result (
  status INT,
  duration INT,
  stdout TEXT,
  stderr TEXT
);

CREATE TABLE test (
  test_name TEXT,
  execution_result_id INT
);

CREATE TABLE mutation_point (
  mutation_operator TEXT,
  module_name TEXT,
  function_name TEXT,
  function_index INT,
  basic_block_index INT,
  instruction_index INT,
  filename TEXT,
  line_number INT,
  column_number INT,
  unique_id TEXT UNIQUE
);

CREATE TABLE mutation_result (
  execution_result_id INT,
  test_id TEXT,
  mutation_point_id TEXT,
  mutation_distance INT,
  __tmp_caller_path TEXT
);

CREATE TABLE mutation_point_debug (
  filename TEXT,
  line_number INT,
  column_number INT,
  function TEXT,
  basic_block TEXT,
  instruction INT,
  unique_id TEXT UNIQUE
);

CREATE TABLE config (
  project_name TEXT,
  bitcode_paths TEXT,
  mutation_operators TEXT,
  dynamic_libraries TEXT,
  tests TEXT,
  fork INT,
  dry_run INT,
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
