#pragma once

#include "mull/TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Program;

class GoogleTestFinder : public TestFinder {
public:
  std::vector<Test> findTests(Program &program) override;
};

} // namespace mull
