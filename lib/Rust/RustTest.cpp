#include "Rust/RustTest.h"

#include "Rust/RustSymbolDemangling.h"

using namespace mull;
using namespace std;

RustTest::RustTest(std::string name, llvm::Function *function) :
  Test(TK_RustTest), testName(name), function(function) {
}

std::string RustTest::getTestName() {
  return testName;
}

std::string RustTest::getTestDisplayName() {
  auto testName = getTestName();

  auto demangledName = RustSymbolDemangle(testName, true);

  return demangledName;
}

llvm::Function *RustTest::getFunction() {
  return function;
}
