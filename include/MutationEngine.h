#pragma once

namespace llvm {
  class Module;
}

namespace mull {

class MutationPoint;

/// FIXME: This class seems to be redundant
class MutationEngine {
public:
  void applyMutation(llvm::Module *M, MutationPoint &MP);
  void revertMutation(MutationPoint &MP);
};

}
