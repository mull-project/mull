#pragma once

#include "TestFrameworks/TestFinder.h"

#include <vector>

namespace mull {

class Context;
class Filter;

class GoogleTestFinder : public TestFinder {
public:
  std::vector<std::unique_ptr<Test>> findTests(Context &context,
                                               Filter &filter) override;
};

}
