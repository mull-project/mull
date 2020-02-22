#pragma once

#include "TestFramework.h"

namespace mull {

struct Configuration;
class Toolchain;
class Diagnostics;

class TestFrameworkFactory {
public:
  TestFramework createTestFramework(const std::string &name, Toolchain &toolchain,
                                    Configuration &configuration, Diagnostics &diagnostics);

  TestFramework simpleTestFramework(Toolchain &toolchain, Configuration &configuration,
                                    Diagnostics &diagnostics);
  TestFramework googleTestFramework(Toolchain &toolchain, Configuration &configuration,
                                    Diagnostics &diagnostics);
  TestFramework customTestFramework(Toolchain &toolchain, Configuration &configuration,
                                    Diagnostics &diagnostics);

  std::vector<std::pair<std::string, std::string>> commandLineOptions();
};

} // namespace mull
