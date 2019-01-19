#pragma once

#include "TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Program;
class Filter;
struct CustomTestDefinition;

class CustomTestFinder : public TestFinder {
  const std::vector<CustomTestDefinition> &testDefinitions;

public:
  explicit CustomTestFinder(
      const std::vector<CustomTestDefinition> &definitions);
  std::vector<std::unique_ptr<Test>> findTests(Program &program,
                                               Filter &filter) override;
};

} // namespace mull
