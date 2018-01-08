#include "Instrumentation/Instrumentation.h"
#include "Test.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

using namespace mull;
using namespace llvm;

Instrumentation::Instrumentation(Toolchain &toolchain)
: callbacks(toolchain), functions() {
  CallTreeFunction phonyRoot(nullptr);
  functions.push_back(phonyRoot);
}

void Instrumentation::insertCallbacks(llvm::Module *originalModule,
                                      llvm::Module *instrumentedModule) {

  for (auto &function: originalModule->getFunctionList()) {
    if (function.isDeclaration()) {
      continue;
    }
    CallTreeFunction callTreeFunction(&function);
    uint64_t index = functions.size();
    functions.push_back(callTreeFunction);
    auto clonedFunction = instrumentedModule->getFunction(function.getName());
    callbacks.injectCallbacks(clonedFunction, index);
  }
}

std::vector<std::unique_ptr<Testee>>
Instrumentation::getTestees(Test *test, Filter &filter, int distance) {
  return test->getInstrumentationInfo().getTestees(functions, test, filter, distance);
}

void Instrumentation::setupInstrumentationInfo(Test *test) {
  test->getInstrumentationInfo().prepare(functions.size());
}

void Instrumentation::cleanupInstrumentationInfo(Test *test) {
  test->getInstrumentationInfo().cleanup(functions.size());
}

