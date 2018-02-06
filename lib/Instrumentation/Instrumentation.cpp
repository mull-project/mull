#include "Instrumentation/Instrumentation.h"
#include "Instrumentation/DynamicCallTree.h"
#include "Test.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <sys/mman.h>
#include <sys/types.h>

using namespace mull;
using namespace llvm;

Instrumentation::Instrumentation()
: callbacks(), functions() {
  CallTreeFunction phonyRoot(nullptr);
  functions.push_back(phonyRoot);
}

const char *Instrumentation::instrumentationInfoVariableName() {
  return "mull_instrumentation_info";
}

void Instrumentation::insertCallbacks(llvm::Module *originalModule,
                                      llvm::Module *instrumentedModule) {
  auto info = callbacks.injectInstrumentationInfoPointer(instrumentedModule,
                                                         instrumentationInfoVariableName());

  for (auto &function: originalModule->getFunctionList()) {
    if (function.isDeclaration()) {
      continue;
    }
    CallTreeFunction callTreeFunction(&function);
    uint64_t index = functions.size();
    functions.push_back(callTreeFunction);
    auto clonedFunction = instrumentedModule->getFunction(function.getName());
    callbacks.injectCallbacks(clonedFunction, index, info);
  }
}

std::vector<std::unique_ptr<Testee>>
Instrumentation::getTestees(Test *test, Filter &filter, int distance) {
  auto &mapping = test->getInstrumentationInfo().callTreeMapping;

  auto callTree = DynamicCallTree::createCallTree(mapping, functions);
  auto subtrees = DynamicCallTree::extractTestSubtrees(callTree.get(), test);
  auto testees = DynamicCallTree::createTestees(subtrees, test, distance, filter);

  return testees;
}

void Instrumentation::setupInstrumentationInfo(Test *test) {
  auto &mapping = test->getInstrumentationInfo().callTreeMapping;

  assert(mapping == nullptr && "Called twice?");
  assert(functions.size() > 1 && "Functions must be filled in before this call");

  auto mappingSize = sizeof(mapping[0]) * functions.size();
  /// Creating a memory to be shared between child and parent.
  auto rawMemory = mmap(NULL, mappingSize,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS,
                        -1, 0);
  mapping = static_cast<uint64_t *>(rawMemory);
  memset(mapping, 0, mappingSize);
}

void Instrumentation::cleanupInstrumentationInfo(Test *test) {
  std::stack<uint64_t>().swap(test->getInstrumentationInfo().callstack);
  munmap(test->getInstrumentationInfo().callTreeMapping, functions.size());
}

