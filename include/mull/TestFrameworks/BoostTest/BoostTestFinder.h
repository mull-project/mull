#pragma once

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Program;

class BoostTestFinder : public TestFinder {
public:
  BoostTestFinder(Diagnostics &diagnostics)
    : diagnostics(diagnostics) {}

  std::vector<Test> findTests(Program &program) override;

private:
  Diagnostics &diagnostics;
};

} // namespace mull
