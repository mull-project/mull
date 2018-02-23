#include <string>
#include <vector>
#include <memory>

namespace mull {

class Result;
class Config;
class Metrics;

class SQLiteReporter {

private:
  std::string databasePath;

public:
  SQLiteReporter(const std::string &projectName = std::string(""));

  void reportResults(const Result &result,
                     const Config &config,
                     const Metrics &metrics);

  std::string getDatabasePath();
};

}
