#pragma once

#include <string>
#include <vector>

namespace llvm {
  class Function;
  class Value;
  class Module;
}

namespace Mutang {

class MutationPoint;
class MutationPointAddress;

class MutationOperator {
public:
  virtual std::vector<MutationPoint *> getMutationPoints(llvm::Function *function) = 0;

  virtual std::string uniqueID() = 0;
  virtual std::string uniqueID() const = 0;

  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) = 0;
  virtual llvm::Value *revertMutation(llvm::Value &Value) = 0;
  virtual ~MutationOperator() {}
};

}
