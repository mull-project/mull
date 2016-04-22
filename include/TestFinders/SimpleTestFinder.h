#pragma once 

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Function.h"

namespace llvm {
  class Module;
}

namespace Mutang {

class Context;

// Finds all methods that start with "test_"
class SimpleTestFinder {
  Context &Ctx;
public:
  explicit SimpleTestFinder(Context &C) : Ctx(C) {}

  llvm::ArrayRef<llvm::Function *> findTests();
  llvm::ArrayRef<llvm::Function *> findTestees(llvm::Function &F);
};

}
