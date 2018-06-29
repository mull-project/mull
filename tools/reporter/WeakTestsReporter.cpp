#include "WeakTestsReporter.h"
#include "ExecutionResult.h"

#include <algorithm>
#include <sqlite3.h>
#include <string>
#include <stdlib.h>
#include <utility>
#include <vector>
#include <map>
#include <set>

struct Location {
  std::string file;
  int32_t line;
  int32_t column;

  Location() = delete;

  Location(std::string file, int line, int column)
    : file(std::move(file)), line(line), column(column) {}

  std::string asString() {
    return file + ":" + std::to_string(line) + ":" + std::to_string(column);
  }
};

struct Mutant {
  std::string mutantId;
  mull::ExecutionStatus status;
  int64_t duration;
  std::string mutator;
  Location location;
  std::string description;

  Mutant() = delete;

  Mutant(std::string mutantId, mull::ExecutionStatus status,
         int64_t duration, std::string mutator, Location location,
         std::string description)
    : mutantId(std::move(mutantId)), status(status), duration(duration),
      mutator(std::move(std::move(mutator))), location(std::move(location)),
      description(std::move(description)) {}
};

class TestResult {
public:
  TestResult(std::string testId, Location location)
    : testId(std::move(testId)), testLocation(std::move(location)) {}

  void addMutant(Mutant mutant) {
    if (mutant.status == mull::ExecutionStatus::Passed) {
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

  size_t survivedMutantsCount() {
    return survivedMutants.size();
  }

  size_t killedMutantsCount() {
    return killedMutants.size();
  }

  size_t totalMutantsCount() {
    return survivedMutantsCount() + killedMutantsCount();
  }

  std::vector<Mutant> getSurvivedMutants() {
    return survivedMutants;
  }

  int mutationScore() const {
    if (killedMutants.empty()) {
      return 0;
    }

    double totalMutants = survivedMutants.size() + killedMutants.size();

    double score = killedMutants.size() / totalMutants;
    return static_cast<int>(score * 100);
  }

private:
  std::string testId;
  Location testLocation;
  std::vector<Mutant> survivedMutants;
  std::vector<Mutant> killedMutants;
};

static std::set<std::string> fetchKilledMutants(const std::string &reportPath) {
  sqlite3 *database;
  sqlite3_open(reportPath.c_str(), &database);
  const char *query = R"query(
  select mutation_point_id from execution_result
  where
  mutation_point_id <> "" and status <> 2
  group by mutation_point_id;
)query";

  std::string selectQuery(query);

  sqlite3_stmt *selectStatement;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.length(),
                  &selectStatement, nullptr);

  std::set<std::string> killedMutants;

  while (true) {
    int stepResult = sqlite3_step(selectStatement);

    if (stepResult == SQLITE_ROW) {
      std::string mutantId(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, 0)));
      killedMutants.insert(mutantId);

    } else if (stepResult == SQLITE_DONE) {
      break;
    } else {
      perror(sqlite3_errmsg(database));
      sqlite3_close(database);
      exit(1);
    }
  }

  sqlite3_close(database);
  return killedMutants;
}

static std::vector<TestResult>
readReportFromSQLite(const std::string &reportPath,
                     std::set<std::string> &killedMutants) {
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
    mp.column_number as mutant_location_column,
    mp.diagnostics as mutant_description
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

  std::string selectQuery(query);

  sqlite3_stmt *selectStatement;
  sqlite3_prepare(database, selectQuery.c_str(), selectQuery.length(),
                  &selectStatement, nullptr);
  std::map<std::string, TestResult> results;

  while (true) {
    int stepResult = sqlite3_step(selectStatement);

    if (stepResult == SQLITE_ROW) {
      int column = 0;
      std::string testId(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement,
                                                           column++)));
      std::string testLocationFile(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement,
                                                           column++)));
      int32_t testLocationLine = sqlite3_column_int(selectStatement, column++);
      Location testLocation(testLocationFile, testLocationLine, 0);

      std::string mutantId(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement,
                                                           column++)));
      auto mutantStatus = static_cast<mull::ExecutionStatus>(sqlite3_column_int(
        selectStatement, column++));

      /// Check if the mutant was killed by another test and
      /// override its status
      if (killedMutants.find(mutantId) != killedMutants.end()) {
        mutantStatus = mull::ExecutionStatus::Failed;
      }
      int64_t mutantDuration = sqlite3_column_int64(selectStatement, column++);
      std::string mutantMutator(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement,
                                                           column++)));

      std::string mutantLocationFile(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement,
                                                           column++)));
      int32_t mutantLocationLine = sqlite3_column_int(selectStatement,
                                                      column++);
      int32_t mutantLocationColumn = sqlite3_column_int(selectStatement,
                                                        column++);
      Location mutantLocation(mutantLocationFile, mutantLocationLine,
                              mutantLocationColumn);
      std::string mutantDescription(
        reinterpret_cast<const char *>(sqlite3_column_text(selectStatement,
                                                           column++)));

      Mutant mutant(mutantId, mutantStatus, mutantDuration, mutantMutator,
                    mutantLocation, mutantDescription);

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
  testResults.reserve(results.size());
  for (auto pair : results) {
    testResults.push_back(pair.second);
  }

  return testResults;
}

void mull::WeakTestsReporter::showReport(const char *reportPath, int lowerBound,
                                         bool includeMutants) {
  auto killedMutants = fetchKilledMutants(reportPath);
  auto results = readReportFromSQLite(reportPath, killedMutants);
  std::sort(results.begin(), results.end(), [](const TestResult &a, const TestResult &b) {
    return a.mutationScore() < b.mutationScore();
  });

  for (auto result : results) {
    auto score = result.mutationScore();
    if (score > lowerBound) {
      continue;
    }

    std::string testLocation = result.getTestLocation().asString();
    printf("%s:", testLocation.c_str());
    printf(" %s %d%%", result.getTestId().c_str(), score);
    printf(" %lu/%lu", result.survivedMutantsCount(), result.totalMutantsCount());
    printf("\n");

    if (includeMutants) {
      for (auto &mutant : result.getSurvivedMutants()) {
        const char *padding = "  ";
        printf("%s%s\n", padding, mutant.location.asString().c_str());
        printf("%s%s\n", padding, mutant.mutantId.c_str());
        printf("%s%s", padding, mutant.description.c_str());
        printf("\n");
      }
      printf("\n");
    }
  }
}
