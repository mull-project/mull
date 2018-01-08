#include "Instrumentation/Instrumentation.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

using namespace mull;
using namespace llvm;

Instrumentation::Instrumentation(Toolchain &toolchain)
: callbacks(toolchain), functions(), instrumentationInfo() {
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
    callbacks.injectCallbacks(clonedFunction, index, instrumentationInfo);
  }
}

std::vector<std::unique_ptr<Testee>>
Instrumentation::getTestees(Test *test, Filter &filter, int distance) {
  return instrumentationInfo.getTestees(functions, test, filter, distance);
}

void Instrumentation::prepare() {
  instrumentationInfo.prepare(functions.size());
}

void Instrumentation::reset() {
  instrumentationInfo.reset();
}
