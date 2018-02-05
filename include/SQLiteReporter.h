#include "Result.h"

#include <string>
#include <vector>
#include <memory>

namespace mull {

class Result;
class Config;
struct MetricsMeasure;

class SQLiteReporter {

private:
  std::string databasePath;

public:
  SQLiteReporter(const std::string &projectName = std::string(""));

  void reportResults(const std::unique_ptr<Result> &result,
                     const Config &config,
                     const MetricsMeasure &resultTime);

  std::string getDatabasePath();
};

}
