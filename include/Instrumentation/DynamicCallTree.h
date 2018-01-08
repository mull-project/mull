#pragma once

#include "Filter.h"

#include <list>
#include <stack>
#include <vector>

#include <llvm/IR/Function.h>

namespace mull {

  class Test;
  class Testee;

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
    DynamicCallTree();

    void prepare(uint64_t *m);
    std::unique_ptr<CallTree> createCallTree(std::vector<CallTreeFunction> functions);
    std::vector<CallTree *> extractTestSubtrees(CallTree *root, Test *test);
    std::vector<std::unique_ptr<Testee>> createTestees(std::vector<CallTree *> subtrees,
                                                       Test *test,
                                                       int distance,
                                                       Filter &filter);

    static void enterFunction(const uint64_t functionIndex,
                              uint64_t *mapping,
                              std::stack<uint64_t> &stack);
    static void leaveFunction(const uint64_t functionIndex,
                              uint64_t *mapping,
                              std::stack<uint64_t> &stack);
  private:
    uint64_t *mapping;
  };

}
