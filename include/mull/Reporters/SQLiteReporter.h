#include "Reporter.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mull {

class Result;
class Diagnostics;

class SQLiteReporter : public Reporter {
public:
  explicit SQLiteReporter(Diagnostics &diagnostics, const std::string &reportDir = "",
                          const std::string &reportName = "",
                          std::unordered_map<std::string, std::string> mullInformation = {});

  void reportResults(const Result &result) override;

  std::string getDatabasePath();

private:
  Diagnostics &diagnostics;
  std::string databasePath;
  std::unordered_map<std::string, std::string> mullInformation;
};

} // namespace mull
