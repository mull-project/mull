#pragma once

#include "Test.h"

#include <memory>

namespace mull {

class Context;
class Filter;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests(Context &context,
                                                       Filter &filter) = 0;
  virtual ~TestFinder() {}
};

}
