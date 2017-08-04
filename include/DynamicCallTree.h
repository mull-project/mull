#pragma once

#include <list>
#include <vector>

#include <llvm/IR/Function.h>

namespace mull {

  struct CallTree {
    llvm::Function *function;
    int level;
    uint64_t functionsIndex;
    std::list<std::unique_ptr<CallTree>> children;
    CallTree(llvm::Function *f) : function(f), level(0), functionsIndex(0) {}
  };

  struct CallTreeFunction {
    llvm::Function *function;
    CallTree *treeRoot;

    CallTreeFunction(llvm::Function *f) : function(f), treeRoot(nullptr) {}
  };

  class DynamicCallTree {
  public:
    DynamicCallTree(uint64_t *m, std::vector<CallTreeFunction> &f);

    std::unique_ptr<CallTree> createCallTree();
    void cleanupCallTree(std::unique_ptr<CallTree> root);
  private:
    uint64_t *mapping;
    std::vector<CallTreeFunction> &functions;
  };

}
