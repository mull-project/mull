#include "DynamicCallTree.h"
#include "Test.h"
#include "Testee.h"

#include <queue>
#include <stack>

using namespace mull;
using namespace llvm;

void DynamicCallTree::enterFunction(const uint64_t functionIndex,
                                    uint64_t *mapping,
                                    std::stack<uint64_t> &stack) {

  if (stack.empty()) {
    /// This is the first function in a chain
    /// The root of a tree
    mapping[functionIndex] = functionIndex;
  } else if (mapping[functionIndex] == 0) {
    /// This function has never been called
    uint64_t parent = stack.top();
    mapping[functionIndex] = parent;
  }

  stack.push(functionIndex);
}

void DynamicCallTree::leaveFunction(const uint64_t functionIndex,
                                    uint64_t *mapping,
                                    std::stack<uint64_t> &stack) {
  stack.pop();
}

void fillInCallTree(std::vector<CallTreeFunction> &functions,
                    uint64_t *callTreeMapping, uint64_t functionIndex) {
  uint64_t parent = callTreeMapping[functionIndex];
  if (parent == 0) {
    return;
  }

  if (parent == functionIndex) {
    parent = 0;
  }

  CallTreeFunction &function = functions[functionIndex];
  std::unique_ptr<CallTree> node = make_unique<CallTree>(function.function);
  function.treeRoot = node.get();

  fillInCallTree(functions, callTreeMapping, parent);

  CallTreeFunction root = functions[parent];
  assert(root.treeRoot);
  node->level = root.treeRoot->level + 1;
  node->functionsIndex = functionIndex;
  root.treeRoot->children.push_back(std::move(node));
  callTreeMapping[functionIndex] = 0;
}

DynamicCallTree::DynamicCallTree(std::vector<CallTreeFunction> &f)
: mapping(nullptr), functions(f) {}

void DynamicCallTree::prepare(uint64_t *m) {
  assert(m != nullptr);
  assert(mapping == nullptr);
  mapping = m;
}

std::unique_ptr<CallTree> DynamicCallTree::createCallTree() {
  assert(mapping != nullptr);
  assert(mapping[0] == 0);
  assert(!functions.empty());
  assert(functions.begin()->function == nullptr);
  assert(functions.begin()->treeRoot == nullptr);

  ///
  /// Building the Call Tree
  ///
  /// To build a call tree we insert callbacks into each function during JIT
  /// execution. The callbacks are called within unique function id.
  /// The callbacks then store information about program execution in a plain
  /// array of function IDs (_callTreeMapping).
  /// The very first element of the array (callTreeMapping[0]) is
  /// zero and not used.
  /// Each subsequent element may have three states:
  ///
  ///   1. If a function N was never called then _callTreeMapping[N] == 0
  ///   2. If a function N was called as a very first function
  ///   (i.e. callstack is empty) then _callTreeMapping[N] == N.
  ///   3. If a function N is called by some other function
  ///   (i.e. callstack is not empty) then _callTreeMapping[N] == callstack.top()
  ///
  /// When the execution is done we can construct a tree of a  more classic
  /// form.
  ///

  std::unique_ptr<CallTree> phonyRoot = make_unique<CallTree>(nullptr);
  CallTreeFunction &rootFunction = functions[0];
  rootFunction.treeRoot = phonyRoot.get();

  for (uint64_t index = 1; index < functions.size(); index++) {
    fillInCallTree(functions, mapping, index);
  }

  return phonyRoot;
}

void DynamicCallTree::cleanupCallTree(std::unique_ptr<CallTree> root) {
  std::queue<CallTree *> nodes;
  nodes.push(root.get());

  while (!nodes.empty()) {
    CallTree *node = nodes.front();
    nodes.pop();

    CallTreeFunction &function = functions[node->functionsIndex];
    function.treeRoot = nullptr;

    for (std::unique_ptr<CallTree> &child : node->children) {
      nodes.push(child.get());
    }
  }
}

std::vector<CallTree *> DynamicCallTree::extractTestSubtrees(CallTree *root,
                                                             Test *test) {
  std::vector<CallTree *> subtrees;
  std::vector<Function *> entryPoints = test->entryPoints();

  std::queue<CallTree *> nodes;
  nodes.push(root);
  while (!nodes.empty()) {
    CallTree *node = nodes.front();
    nodes.pop();

    if (std::find(entryPoints.begin(), entryPoints.end(), node->function) != entryPoints.end()) {
      subtrees.push_back(node);
    }

    for (auto &child : node->children) {
      nodes.push(child.get());
    }
  }
  return subtrees;
}

std::vector<std::unique_ptr<Testee>>
DynamicCallTree::createTestees(std::vector<CallTree *> subtrees,
                               Test *test,
                               int maxDistance,
                               Filter &filter) {
  std::vector<std::unique_ptr<Testee>> testees;

  for (CallTree *root : subtrees) {
    const int offset = root->level;

    std::queue<CallTree *> nodes;
    nodes.push(root);

    Testee *parent = nullptr;

    while (!nodes.empty()) {
      CallTree *node = nodes.front();
      nodes.pop();

      if (filter.shouldSkipFunction(node->function)) {
        continue;
      }

      int distance = node->level - offset;
      std::unique_ptr<Testee> testee(make_unique<Testee>(node->function,
                                                         nullptr,
                                                         parent,
                                                         distance));
      parent = testee.get();
      testees.push_back(std::move(testee));
      if (distance < maxDistance) {
        for (std::unique_ptr<CallTree> &child : node->children) {
          nodes.push(child.get());
        }
      }
    }
  }

  return testees;
}
