#pragma once

#include "TestFinder.h"
#include "TestRunner.h"

namespace mull {

class TestFramework {
public:
  TestFramework(std::unique_ptr<TestFinder> finder,
                std::unique_ptr<TestRunner> runner);
  ~TestFramework() = default;

  TestFinder &finder();
  TestRunner &runner();

private:
  std::unique_ptr<TestFinder> _finder;
  std::unique_ptr<TestRunner> _runner;
};

} // namespace mull
