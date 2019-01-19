#pragma once

#include "TestFramework.h"

namespace mull {

struct Configuration;
class Toolchain;

class TestFrameworkFactory {
public:
  TestFrameworkFactory(Configuration &configuration, Toolchain &toolchain);
  TestFramework createTestFramework(const std::string &name);

  TestFramework simpleTestFramework();
  TestFramework googleTestFramework();
  TestFramework customTestFramework();

private:
  Configuration &configuration;
  Toolchain &toolchain;
};

} // namespace mull
