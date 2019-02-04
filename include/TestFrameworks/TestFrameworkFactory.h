#pragma once

#include "TestFramework.h"

namespace mull {

struct Configuration;
class Toolchain;

class TestFrameworkFactory {
public:
  TestFramework createTestFramework(const std::string &name,
                                    Toolchain &toolchain,
                                    Configuration &configuration);

  TestFramework simpleTestFramework(Toolchain &toolchain,
                                    Configuration &configuration);
  TestFramework googleTestFramework(Toolchain &toolchain,
                                    Configuration &configuration);
  TestFramework customTestFramework(Toolchain &toolchain,
                                    Configuration &configuration);

  std::vector<std::pair<std::string, std::string>> commandLineOptions();
};

} // namespace mull
