#pragma once

#include <list>
#include <stack>
#include <vector>

#include <llvm/IR/Function.h>

namespace mull {

class Test;
class ReachableFunction;

struct CallTree {
  llvm::Function *function;
  int level;
  uint32_t functionsIndex;
  std::list<std::unique_ptr<CallTree>> children;
  CallTree(llvm::Function *f) : function(f), level(0), functionsIndex(0) {}
};

struct CallTreeFunction {
  llvm::Function *function;
  CallTree *treeRoot;

  CallTreeFunction(llvm::Function *f) : function(f), treeRoot(nullptr) {}
};

/// TODO: What is the good practice for this? maybe namespace?
class DynamicCallTree {
public:
  DynamicCallTree() = delete;
  ~DynamicCallTree() = delete;

  static std::unique_ptr<CallTree>
  createCallTree(uint32_t *mapping, std::vector<CallTreeFunction> functions);
  static std::vector<CallTree *> extractTestSubtrees(CallTree *root,
                                                     Test &test);
  static std::vector<std::unique_ptr<ReachableFunction>>
  createReachableFunctions(std::vector<CallTree *> subtrees, Test &test,
                           int maxDistance);

  static void enterFunction(uint32_t functionIndex, uint32_t *mapping,
                            std::stack<uint32_t> &stack);
  static void leaveFunction(uint32_t functionIndex, uint32_t *mapping,
                            std::stack<uint32_t> &stack);
};

} // namespace mull
