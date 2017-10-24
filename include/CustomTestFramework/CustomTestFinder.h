#pragma once

#include "TestFinder.h"

#include <vector>

namespace mull {

class Context;
class Filter;
struct CustomTestDefinition;

class CustomTestFinder : public TestFinder {
  const std::vector<CustomTestDefinition> &testDefinitions;
public:
  CustomTestFinder(const std::vector<CustomTestDefinition> &definitions);
  std::vector<std::unique_ptr<Test>> findTests(Context &context,
                                               Filter &filter) override;
};

}