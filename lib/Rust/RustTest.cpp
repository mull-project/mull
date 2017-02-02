#include "Rust/RustTest.h"

using namespace mull;
using namespace std;

RustTest::RustTest(std::string name, llvm::Function *function) :
  Test(TK_RustTest), testName(name), function(function) {
}

std::string RustTest::getTestName() {
  return testName;
}

llvm::Function *RustTest::getFunction() {
  return function;
}
