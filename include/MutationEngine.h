#pragma once

namespace llvm {
  class Module;
}

namespace Mutang {

class MutationPoint;

/// FIXME: This class seems to be redundant
class MutationEngine {
public:
  void applyMutation(MutationPoint &MP);
  void revertMutation(MutationPoint &MP);
};

}
