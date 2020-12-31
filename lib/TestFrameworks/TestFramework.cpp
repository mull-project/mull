#include "mull/TestFrameworks/TestFramework.h"

using namespace mull;

TestFramework::TestFramework(std::unique_ptr<mull::TestFinder> finder)
    : _finder(std::move(finder)) {}

TestFramework::TestFramework(TestFramework &&other) noexcept : _finder(std::move(other._finder)) {}

TestFinder &TestFramework::finder() { return *_finder; }
