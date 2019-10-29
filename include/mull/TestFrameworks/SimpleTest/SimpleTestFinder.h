#pragma once

#include "mull/TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Program;

class SimpleTestFinder : public TestFinder {
public:
  // Finds all methods that start with "test_"
  std::vector<Test> findTests(Program &program) override;
};

} // namespace mull
