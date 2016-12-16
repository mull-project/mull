#include "llvm/IR/Function.h"

namespace Mutang {

  class Testee {
    llvm::Function *function;
    llvm::Function *parentFunction;
    int distance;

  public:
    Testee(llvm::Function *function,
           llvm::Function *parentFunction,
           int distance) : function(function),
                           parentFunction(parentFunction),
                           distance(distance) {}

    llvm::Function *getFunction() const {
      return function;
    }

    llvm::Function *getParentFunction() const {
      return parentFunction;
    }

    int getDistance() const {
      return distance;
    }
  };
}
