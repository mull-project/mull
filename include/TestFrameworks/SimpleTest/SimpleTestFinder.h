#pragma once

#include "TestFrameworks/TestFinder.h"
#include "TestFrameworks/Test.h"

#include <vector>

namespace mull {

class Context;
class Filter;

class SimpleTestFinder : public TestFinder {
public:
  // Finds all methods that start with "test_"
  std::vector<std::unique_ptr<Test>> findTests(Context &context,
                                               Filter &filter) override;
};

}
