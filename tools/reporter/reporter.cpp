#include "ExecutionResult.h"

#include <llvm/Support/CommandLine.h>
#include <sqlite3.h>
#include <map>

using namespace mull;
using namespace llvm;

void mull_reporter_version() {
  printf("Helo?\n");
}

cl::OptionCategory MullOptionCategory("mull-reporter");

static cl::opt<std::string> SqliteReportFile(cl::desc("<sqlite report>"),
                                             cl::Positional,
                                             cl::cat(MullOptionCategory),
                                             cl::Required);

static cl::opt<int> LowerBound("lower_bound",
                               cl::desc("Lower bound for mutation score (0-100)"),
                               cl::value_desc("mutation score"),
                               cl::cat(MullOptionCategory),
                               cl::Required);

static cl::opt<int> UpperBound("upper_bound",
                               cl::desc("Upper bound for mutation score (0-100)"),
                               cl::value_desc("mutation score"),
                               cl::cat(MullOptionCategory),
                               cl::Required);

struct Location {
  std::string file;
  int32_t line;
  int32_t column;
  Location() = delete;
  Location(std::string file, int line, int column)
  : file(file), line(line), column(column) {}
  std::string asString() {
    return file + ":" + std::to_string(line) + ":" + std::to_string(column);
  }
};

struct Mutant {
  std::string mutantId;
  ExecutionStatus status;
  int64_t duration;
  std::string mutator;
  Location location;
  Mutant() = delete;
  Mutant(std::string mutantId, ExecutionStatus status,
         int64_t duration, std::string mutator, Location location)
  : mutantId(mutantId), status(status), duration(duration),
    mutator(mutator), location(location) {}
};

class TestResult {
public:
  TestResult(std::string testId, Location location)
  : testId(testId), testLocation(location) {}
  void addMutant(Mutant mutant) {
    if (mutant.status == ExecutionStatus::Passed) {
      survivedMutants.push_back(mutant);
    } else {
      killedMutants.push_back(mutant);
    }
  }

  std::string &getTestId() {
    return testId;
  }

  Location &getTestLocation() {
    return testLocation;
  }

  std::vector<Mutant> getSurvivedMutants() {
    return survivedMutants;
  }

  int mutationScore() {
    if (killedMutants.size() == 0) {
      return 0;
    }

    double totalMutants = survivedMutants.size() + killedMutants.size();

    double score = killedMutants.size() / totalMutants;
    return score * 100;
  }
private:
  std::string testId;
  Location testLocation;
  std::vector<Mutant> survivedMutants;
  std::vector<Mutant> killedMutants;
};

struct {
  bool operator()(TestResult &a, TestResult &b) const
  {
    return a.mutationScore() < b.mutationScore();
  }
} TestResultComparator;

static std::vector<TestResult> readReportFromSQLite(const std::string &reportPath) {
  sqlite3 *database;
  sqlite3_open(reportPath.c_str(), &database);

  const char *query = R"query(
  select
    t.unique_id as test_id,
    t.location_file as test_location_file,
    t.location_line as test_location_line,
    ex.mutation_point_id as mutant_id,
    ex.status as status,
    ex.duration as duration,
    mp.mutator as mutator,
    mp.filename as mutant_location_file,
    mp.line_number as mutant_location_line,
    mp.column_number as mutant_location_column
  from test as t
  join
    execution_result as ex
    on t.unique_id = ex.test_id
  join
    mutation_point as mp
    on ex.mutation_point_id = mp.unique_id
  where mutation_point_id <> ""
  order by test_id;
)query";

  std::string selectQuery = "";

  sqlite3_stmt *selectStatement;
  sqlite3_prepare(database, query, strlen(query), &selectStatement, NULL);
  std::map<std::string, TestResult> results;

  while (1) {
    int stepResult = sqlite3_step(selectStatement);

    if (stepResult == SQLITE_ROW) {
      int column = 0;
      std::string testId(reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, column++)));
      std::string testLocationFile(reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, column++)));
      int32_t testLocationLine = sqlite3_column_int(selectStatement, column++);
      Location testLocation(testLocationFile, testLocationLine, 0);

      std::string mutantId(reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, column++)));
      ExecutionStatus mutantStatus = static_cast<ExecutionStatus>(sqlite3_column_int(selectStatement, column++));
      int64_t mutantDuration = sqlite3_column_int64(selectStatement, column++);
      std::string mutantMutator(reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, column++)));

      std::string mutantLocationFile(reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, column++)));
      int32_t mutantLocationLine = sqlite3_column_int(selectStatement, column++);
      int32_t mutantLocationColumn = sqlite3_column_int(selectStatement, column++);
      Location mutantLocation(mutantLocationFile, mutantLocationLine, mutantLocationColumn);

      Mutant mutant(mutantId, mutantStatus, mutantDuration, mutantMutator, mutantLocation);

      if (results.count(testId) != 0) {
        TestResult &result = results.at(testId);
        result.addMutant(mutant);
      } else {
        TestResult result(testId, testLocation);
        result.addMutant(mutant);
        results.insert(std::make_pair(testId, result));
      }

    } else if (stepResult == SQLITE_DONE) {
      break;
    } else {
      perror(sqlite3_errmsg(database));
      sqlite3_close(database);
      exit(1);
    }
  }

  sqlite3_close(database);

  std::vector<TestResult> testResults;
  for (auto pair : results) {
    testResults.push_back(pair.second);
  }

  return testResults;
}

static void printTestResult(TestResult &result, std::string type) {
  std::string testLocation = result.getTestLocation().asString();
  printf("%s: %s: ", testLocation.c_str(), type.c_str());
  printf("%s %d%%\n", result.getTestId().c_str(), result.mutationScore());

  for (auto mutant : result.getSurvivedMutants()) {
    printf("%s: note: ", mutant.location.asString().c_str());
    printf("%s\n", mutant.mutator.c_str());
  }
}

static void printAsError(TestResult &result) {
  printTestResult(result, "error");
}

static void printAsWarning(TestResult &result) {
  printTestResult(result, "warning");
}

static void printDiagnostics(const std::vector<TestResult> &results,
                             int lowerBound, int upperBound) {
  for (auto result : results) {
    int mutationScore = result.mutationScore();
    if (mutationScore > lowerBound && mutationScore <= upperBound) {
      printAsWarning(result);
    } else if (mutationScore <= lowerBound) {
      printAsError(result);
    }
  }
}

int main(int argc, char *argv[]) {
  cl::SetVersionPrinter(mull_reporter_version);
  cl::HideUnrelatedOptions(MullOptionCategory);
  cl::ParseCommandLineOptions(argc, argv, "mull-reporter");

  auto results = readReportFromSQLite(SqliteReportFile);
  std::sort(results.begin(), results.end(), TestResultComparator);
  printDiagnostics(results, LowerBound, UpperBound);

  return 0;
}

