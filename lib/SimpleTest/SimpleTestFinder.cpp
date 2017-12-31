#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTest_Test.h"
#include "Context.h"
#include "Logger.h"
#include "Filter.h"

#include <llvm/IR/Module.h>

#include <vector>

using namespace mull;
using namespace llvm;

std::vector<std::unique_ptr<Test>> SimpleTestFinder::findTests(Context &context,
                                                               Filter &filter) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &module : context.getModules()) {
    auto &x = module.getModule()->getFunctionList();
    for (auto &Fn : x) {

      /// We find C functions having test_ and the same functions if they are
      /// compiled with C++ (mangled as "_Z25test_").
      if (Fn.getName().find("test_") != std::string::npos) {

        Logger::info() << "SimpleTestFinder::findTests - found function "
                       << Fn.getName() << '\n';

        tests.emplace_back(make_unique<SimpleTest_Test>(&Fn));
      }
    }
  }

  return tests;
}
