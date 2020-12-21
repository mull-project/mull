#pragma once

#include <string>
#include <vector>

namespace mull {

struct Configuration;
class Diagnostics;

class Linker {
public:
  Linker(const Configuration &configuration, Diagnostics &diagnostics);
  std::string linkObjectFiles(const std::vector<std::string> &objects);

private:
  const Configuration &configuration;
  Diagnostics &diagnostics;
};

} // namespace mull
