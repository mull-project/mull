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

void mull_moveFunctionBody(Function &OrigF, ValueToValueMapTy &VMap,
                           ValueMaterializer *Materializer,
                           Function *NewF) {
  assert(!OrigF.isDeclaration() && "Nothing to move");
  if (!NewF)
    NewF = cast<Function>(VMap[&OrigF]);
  else
    assert(VMap[&OrigF] == NewF && "Incorrect function mapping in VMap.");
  assert(NewF && "Function mapping missing from VMap.");
  assert(NewF->getParent() != OrigF.getParent() &&
         "moveFunctionBody should only be used to move bodies between "
         "modules.");

  SmallVector<ReturnInst *, 8> Returns; // Ignore returns cloned.
  CloneFunctionInto(NewF, &OrigF, VMap, /*ModuleLevelChanges=*/true, Returns,
                    "", nullptr, nullptr, Materializer);
}

}

