#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"

#include "mull/Filter.h"
#include "mull/Logger.h"
#include "mull/Program/Program.h"

#include <llvm/IR/Module.h>

#include <vector>

using namespace mull;
using namespace llvm;

std::vector<Test> SimpleTestFinder::findTests(Program &program,
                                              Filter &filter) {
  std::vector<Test> tests;

  for (auto &bitcode : program.bitcode()) {
    auto &x = bitcode->getModule()->getFunctionList();
    for (auto &Fn : x) {

      /// We find C functions having test_ and the same functions if they are
      /// compiled with C++ (mangled as "_Z25test_").
      if (Fn.getName().find("test_") != std::string::npos) {

        Logger::info() << "SimpleTestFinder::findTests - found function "
                       << Fn.getName() << '\n';

        tests.push_back(Test(Fn.getName(), "mull", Fn.getName(), {}, &Fn));
      }
    }
  }

  return tests;
}
