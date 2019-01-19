#pragma once

#include "TestFrameworks/Test.h"
#include "TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Program;
class Filter;

class SimpleTestFinder : public TestFinder {
public:
  // Finds all methods that start with "test_"
  std::vector<std::unique_ptr<Test>> findTests(Program &program,
                                               Filter &filter) override;
};

} // namespace mull
