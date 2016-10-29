#pragma once

#include <string>
#include <vector>

static int MutangDefaultTimeout = 3000;

namespace Mutang {

class Config {
  std::vector<std::string> bitcodePaths;
  bool fork;
  int timeout;

public:
  Config(const std::vector<std::string> &paths, bool fork, int timeout) :
    bitcodePaths(paths), fork(fork), timeout(timeout) {}

  const std::vector<std::string> &getBitcodePaths() const {
      return bitcodePaths;
  }

  bool getFork() const {
    return fork;
  }

  float getTimeout() const {
    return timeout;
  }
};

}
