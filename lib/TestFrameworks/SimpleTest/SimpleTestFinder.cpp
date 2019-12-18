#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "mull/Program/Program.h"

using namespace mull;
using namespace llvm;

std::vector<Test> SimpleTestFinder::findTests(Program &program) {
  std::vector<Test> tests;

  for (auto &bitcode : program.bitcode()) {
    for (auto &function : *bitcode->getModule()) {
      /// We find C functions having test_ and the same functions if they are
      /// compiled with C++ (mangled as "_Z25test_").
      if (function.getName().find("test_") != std::string::npos) {
        tests.push_back(Test(function.getName(), "mull", function.getName(), {}, &function));
      }
    }
  }

  return tests;
}
