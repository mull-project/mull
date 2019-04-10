#include "mull/TestFrameworks/CustomTestFramework/CustomTest_Test.h"
#include "mull/Logger.h"

#include <utility>

using namespace mull;
using namespace std;

CustomTest_Test::CustomTest_Test(std::string test, std::string program,
                                 std::vector<std::string> args,
                                 llvm::Function *body)
    : Test(TestKind::TK_CustomTest), testName(std::move(test)),
      programName(std::move(program)), arguments(std::move(args)),
      testFunction(body) {}

std::string CustomTest_Test::getTestName() { return testName; }

std::string CustomTest_Test::getTestDisplayName() { return getTestName(); }