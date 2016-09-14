#pragma once

#include <string>
#include <vector>

namespace Mutang {

class Config {
  std::vector<std::string> BitcodePaths;
  bool fork;
public:
  Config(const std::vector<std::string> &Paths, bool fork) : BitcodePaths(Paths), fork(fork) {}
  const std::vector<std::string> &GetBitcodePaths() const {
      return BitcodePaths;
  }
  bool getFork() const {
    return fork;
  }
};

}
