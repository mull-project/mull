
#include <string>
#include <vector>

namespace Mutang {

class TestResult;

class SQLiteReporter {

private:
  std::string databasePath;

public:
  SQLiteReporter();
  
  void reportResults(const std::vector<std::unique_ptr<TestResult>> &results);
  std::string getDatabasePath();
};

}
