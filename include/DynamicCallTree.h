#pragma once

#include <list>
#include <vector>

#include <llvm/IR/Function.h>

namespace mull {

  struct _CallTree {
    llvm::Function *function;
    int level;
    uint64_t functionsIndex;
    std::list<std::unique_ptr<_CallTree>> children;
    _CallTree(llvm::Function *f) : function(f), level(0), functionsIndex(0) {}
  };

  struct _CallTreeFunction {
    llvm::Function *function;
    _CallTree *treeRoot;

    _CallTreeFunction(llvm::Function *f) : function(f), treeRoot(nullptr) {}
  };

  class DynamicCallTree {
  public:
    DynamicCallTree(uint64_t *m, std::vector<_CallTreeFunction> &f);

    std::unique_ptr<_CallTree> createCallTree();
  private:
    uint64_t *mapping;
    std::vector<_CallTreeFunction> &functions;
  };

}
