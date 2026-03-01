#include "Reporter.h"

#include "mull/ExecutionResult.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct MullDiagnostics;

namespace mull {

class Result;

class SQLiteReporter : public Reporter {
public:
  explicit SQLiteReporter(const MullDiagnostics &diagnostics, const std::string &reportDir = "",
                          const std::string &reportName = "",
                          std::unordered_map<std::string, std::string> mullInformation = {});

  void reportResults(const Result &result) override;

  std::string getDatabasePath();

private:
  const MullDiagnostics &diagnostics;
  std::string databasePath;
  std::unordered_map<std::string, std::string> mullInformation;
};

} // namespace mull
