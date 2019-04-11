#pragma once

#include "mull/TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Program;
class Filter;

class GoogleTestFinder : public TestFinder {
public:
  std::vector<Test> findTests(Program &program, Filter &filter) override;
};

} // namespace mull
