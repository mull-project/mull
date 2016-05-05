#pragma once

namespace llvm {
  class Value;
}

namespace Mutang {

class MutationOperator {
public:
  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *applyMutation(llvm::Value &OriginalValue) = 0;
  virtual llvm::Value *revertMutation(llvm::Value &Value) = 0;
  virtual ~MutationOperator() {}
};

}
