#include "mull/TestFrameworks/CustomTestFramework/CustomTest_Test.h"
#include "mull/Logger.h"

#include <utility>

using namespace mull;
using namespace std;

CustomTest_Test::CustomTest_Test(std::string test, std::string program,
                                 std::vector<std::string> args,
                                 llvm::Function *body,
                                 std::vector<llvm::Function *> constructors)
    : Test(TestKind::TK_CustomTest), testName(std::move(test)),
      programName(std::move(program)), arguments(std::move(args)),
      staticConstructors(std::move(constructors)), testFunction(body) {}

std::string CustomTest_Test::getTestName() { return testName; }

std::string CustomTest_Test::getTestDisplayName() { return getTestName(); }