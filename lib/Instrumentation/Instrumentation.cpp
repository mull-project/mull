#include "mull/Instrumentation/Instrumentation.h"

#include "mull/Instrumentation/DynamicCallTree.h"
#include "mull/TestFrameworks/Test.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <sys/mman.h>
#include <sys/types.h>

using namespace mull;
using namespace llvm;

Instrumentation::Instrumentation() : callbacks(), functions() {
  CallTreeFunction phonyRoot(nullptr);
  functions.push_back(phonyRoot);
}

std::map<std::string, uint32_t> &Instrumentation::getFunctionOffsetMapping() {
  return functionOffsetMapping;
}

const char *Instrumentation::instrumentationInfoVariableName() {
  return "mull_instrumentation_info";
}

const char *Instrumentation::functionIndexOffsetPrefix() {
  return "mull_function_index_offset_";
}

void Instrumentation::recordFunctions(llvm::Module *originalModule) {
  uint32_t offset = functions.size();
  functionOffsetMapping[originalModule->getModuleIdentifier()] = offset;

  for (auto &function : originalModule->getFunctionList()) {
    if (function.isDeclaration()) {
      continue;
    }
    CallTreeFunction callTreeFunction(&function);
    functions.push_back(callTreeFunction);
  }
}

void Instrumentation::insertCallbacks(llvm::Module *instrumentedModule) {
  auto info = callbacks.injectInstrumentationInfoPointer(
      instrumentedModule, instrumentationInfoVariableName());
  auto offset = callbacks.injectFunctionIndexOffset(
      instrumentedModule, functionIndexOffsetPrefix());

  uint32_t index = 0;
  for (auto &function : instrumentedModule->getFunctionList()) {
    if (function.isDeclaration()) {
      continue;
    }
    callbacks.injectCallbacks(&function, index, info, offset);
    index++;
  }
}

std::vector<std::unique_ptr<ReachableFunction>>
Instrumentation::getReachableFunctions(Test &test, int distance) {
  auto &mapping = test.getInstrumentationInfo().callTreeMapping;

  auto callTree = DynamicCallTree::createCallTree(mapping, functions);
  auto subtrees = DynamicCallTree::extractTestSubtrees(callTree.get(), test);
  auto functionsUnderTest =
      DynamicCallTree::createReachableFunctions(subtrees, test, distance);

  return functionsUnderTest;
}

void Instrumentation::setupInstrumentationInfo(Test &test) {
  auto &mapping = test.getInstrumentationInfo().callTreeMapping;

  assert(mapping == nullptr && "Called twice?");
  assert(functions.size() > 1 &&
         "Functions must be filled in before this call");

  auto mappingSize = getMappingSize(test);
  /// Creating a memory to be shared between child and parent.
  auto rawMemory = mmap(nullptr, mappingSize, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  mapping = static_cast<uint32_t *>(rawMemory);
  memset(mapping, 0, mappingSize);
}

void Instrumentation::cleanupInstrumentationInfo(Test &test) {
  std::stack<uint32_t>().swap(test.getInstrumentationInfo().callstack);
  munmap(test.getInstrumentationInfo().callTreeMapping, getMappingSize(test));
}

size_t Instrumentation::getMappingSize(Test &test) {
  return sizeof(test.getInstrumentationInfo().callTreeMapping[0]) * functions.size();
}
