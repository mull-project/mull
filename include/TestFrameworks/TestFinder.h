#pragma once

#include "Test.h"

#include <memory>

namespace mull {

class Program;
class Filter;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests(Program &program,
                                                       Filter &filter) = 0;
  virtual ~TestFinder() = default;
};

} // namespace mull
