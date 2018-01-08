#include "Instrumentation/InstrumentationInfo.h"

#include <sys/mman.h>
#include <sys/types.h>

using namespace mull;

InstrumentationInfo::InstrumentationInfo(std::vector<CallTreeFunction> &f)
: dynamicCallTree(f), _callTreeMapping(nullptr), mappingSize(0), _callstack() {}

InstrumentationInfo::~InstrumentationInfo() {
  munmap(_callTreeMapping, mappingSize);
}

uint64_t *InstrumentationInfo::callTreeMapping() {
  return _callTreeMapping;
}
std::stack<uint64_t> &InstrumentationInfo::callstack() {
  return _callstack;
}

void InstrumentationInfo::reset() {
  _callstack = std::stack<uint64_t>();
  memset(_callTreeMapping, 0, mappingSize * sizeof(_callTreeMapping[0]));
}

void InstrumentationInfo::prepare(size_t mappingSize) {
  assert(_callTreeMapping == nullptr && "Called twice?");
  assert(mappingSize > 1 && "Functions must be filled in before this call");

  /// Creating a memory to be shared between child and parent.
  _callTreeMapping = (uint64_t *) mmap(NULL,
                                       sizeof(_callTreeMapping[0]) * mappingSize,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_ANONYMOUS,
                                       -1,
                                       0);
  memset(_callTreeMapping, 0, mappingSize * sizeof(_callTreeMapping[0]));
  dynamicCallTree.prepare(_callTreeMapping);
}

std::vector<std::unique_ptr<Testee>>
InstrumentationInfo::getTestees(Test *test, Filter &filter, int distance) {
  std::unique_ptr<CallTree> callTree(dynamicCallTree.createCallTree());

  auto subtrees = dynamicCallTree.extractTestSubtrees(callTree.get(), test);
  auto testees = dynamicCallTree.createTestees(subtrees, test, distance, filter);

  dynamicCallTree.cleanupCallTree(std::move(callTree));
  return testees;
}
