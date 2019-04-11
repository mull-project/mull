#pragma once

#include "Test.h"

#include <memory>

namespace mull {

class Program;
class Filter;

class TestFinder {
public:
  virtual std::vector<Test> findTests(Program &program, Filter &filter) = 0;
  virtual ~TestFinder() = default;
};

} // namespace mull
