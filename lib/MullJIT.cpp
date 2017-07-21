#include "MullJIT.h"

namespace mull {

uint64_t *_callTreeMapping = nullptr;
std::stack<uint64_t> _callstack;

extern "C" void mull_enterFunction(uint64_t functionIndex) {
  assert(_callTreeMapping);

  if (_callstack.empty()) {
    /// This is the first function in a chain
    /// The root of a tree
    _callTreeMapping[functionIndex] = functionIndex;
  } else if (_callTreeMapping[functionIndex] == 0) {
    /// This function has never been called
    uint64_t parent = _callstack.top();
    _callTreeMapping[functionIndex] = parent;
  }

  _callstack.push(functionIndex);
}

extern "C" void mull_leaveFunction(uint64_t functionIndex) {
  assert(_callTreeMapping);
  _callstack.pop();
}

}

