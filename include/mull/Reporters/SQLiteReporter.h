#include "Reporter.h"

#include <memory>
#include <string>
#include <vector>

namespace mull {

class Result;
class Metrics;

class SQLiteReporter : public Reporter {
public:
  explicit SQLiteReporter(const std::string &reportDir = "",
                          const std::string &reportName = "");

  void reportResults(const Result &result, const Metrics &metrics) override;

  std::string getDatabasePath();

private:
  std::string databasePath;
};

} // namespace mull
