#pragma once

#include "Test.h"

namespace mull {

class Context;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) = 0;
  virtual ~TestFinder() {}
};

}
