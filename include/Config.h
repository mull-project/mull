#pragma once

#include <string>
#include <vector>

static int MutangDefaultTimeout = 3000;

namespace Mutang {

class Config {
  std::vector<std::string> bitcodePaths;
  bool fork;
  bool dryRun;
  int timeout;

public:
  Config(const std::vector<std::string> &paths, bool fork, bool dryrun, int timeout) :
    bitcodePaths(paths), fork(fork), dryRun(dryrun), timeout(timeout) {}

  const std::vector<std::string> &getBitcodePaths() const {
      return bitcodePaths;
  }

  bool getFork() const {
    return fork;
  }

  int getTimeout() const {
    return timeout;
  }

  bool isDryRun() const {
    return dryRun;
  }

};

}
