
#include <string>
#include <vector>

namespace Mutang {

class Result;

class SQLiteReporter {

private:
  std::string databasePath;

public:
  SQLiteReporter();
  
  void reportResults(const std::unique_ptr<Result> &result);
  std::string getDatabasePath();
};

}
