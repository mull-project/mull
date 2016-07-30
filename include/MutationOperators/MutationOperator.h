#pragma once

namespace llvm {
  class Value;
  class Module;
}

namespace Mutang {

class MutationPointAddress;

class MutationOperator {
public:
  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) = 0;
  virtual llvm::Value *revertMutation(llvm::Value &Value) = 0;
  virtual ~MutationOperator() {}
};

}
