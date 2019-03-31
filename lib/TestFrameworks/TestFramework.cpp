#include "mull/TestFrameworks/TestFramework.h"

using namespace mull;

TestFramework::TestFramework(std::unique_ptr<mull::TestFinder> finder,
                             std::unique_ptr<mull::TestRunner> runner)
    : _finder(std::move(finder)), _runner(std::move(runner)) {}

TestFramework::TestFramework(TestFramework &&other)
    : _finder(std::move(other._finder)), _runner(std::move(other._runner)) {}

TestFinder &TestFramework::finder() { return *_finder; }

TestRunner &TestFramework::runner() { return *_runner; }
