#pragma once

#include "TestFinder.h"

namespace mull {

class TestFramework {
public:
  explicit TestFramework(std::unique_ptr<TestFinder> finder);
  TestFramework(TestFramework &&) noexcept;
  ~TestFramework() = default;

  TestFinder &finder();

private:
  std::unique_ptr<TestFinder> _finder;
};

} // namespace mull
