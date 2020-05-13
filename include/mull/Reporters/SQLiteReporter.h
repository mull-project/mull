#include "Reporter.h"

#include <memory>
#include <string>
#include <vector>

namespace mull {

class Result;
class Diagnostics;

class SQLiteReporter : public Reporter {
public:
  explicit SQLiteReporter(Diagnostics &diagnostics, const std::string &reportDir = "",
                          const std::string &reportName = "");

  void reportResults(const Result &result) override;

  std::string getDatabasePath();

private:
  Diagnostics &diagnostics;
  std::string databasePath;
};

} // namespace mull
