#pragma once

namespace llvm {
  class Value;
}

namespace Mutang {

class MutationOperator {
public:
  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual ~MutationOperator() {}
};

}
