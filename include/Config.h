#pragma once

#include <string>
#include <vector>

static float MutangDefaultTimeout = 3;

namespace Mutang {

class Config {
  std::vector<std::string> BitcodePaths;
  bool fork;
  float timeout;

public:
  Config(const std::vector<std::string> &Paths, bool fork, float timeout) :
    BitcodePaths(Paths), fork(fork), timeout(timeout) {}

  const std::vector<std::string> &GetBitcodePaths() const {
      return BitcodePaths;
  }

  bool getFork() const {
    return fork;
  }

  float getTimeout() const {
    return timeout;
  }
};

}
