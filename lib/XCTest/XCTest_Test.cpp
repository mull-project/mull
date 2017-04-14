#include "XCTest/XCTest_Test.h"

using namespace mull;
using namespace std;

XCTest_Test::XCTest_Test(std::string name, llvm::Function *function) :
Test(TK_XCTest), testName(name), function(function) {
}

std::string XCTest_Test::getTestName() {
  return testName;
}

llvm::Function *XCTest_Test::getFunction() {
  return function;
}
