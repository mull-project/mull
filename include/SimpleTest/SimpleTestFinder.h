#pragma once

#include "TestFinder.h"
#include "Test.h"

#include <map>
#include <vector>

namespace mull {

class Context;

class SimpleTestFinder : public TestFinder {
public:
  // Finds all methods that start with "test_"
  std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) override;
};

}
