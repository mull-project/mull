#pragma once

namespace llvm {
  class Module;
}

namespace Mutang {

class MutationPoint;

class MutationEngine {
public:
  void applyMutation(MutationPoint &MP);
  void revertMutation(MutationPoint &MP);
};

}
