#include "mull/ReachableFunction.h"

namespace mull {
std::vector<FunctionUnderTest> mergeReachableFunctions(
    std::vector<std::unique_ptr<ReachableFunction>> &functions) {
  std::map<llvm::Function *, FunctionUnderTest> functionsMap;

  for (std::unique_ptr<ReachableFunction> &reachable : functions) {
    auto function = reachable->getFunction();
    auto test = reachable->getTest();
    auto distance = reachable->getDistance();
    auto merged = functionsMap.find(function);
    if (merged != functionsMap.end()) {
      merged->second.addReachableTest(test, distance);
    } else {
      functionsMap.insert(std::make_pair(
          function, FunctionUnderTest(function, test, distance)));
    }
  }

  std::vector<FunctionUnderTest> functionsUnderTest;
  for (std::unique_ptr<ReachableFunction> &reachable : functions) {
    /// TODO: We could have just accumulated contents from the map,
    /// But it would break the order of mutants: some of our tests depend on it
    if (functionsMap.empty()) {
      break;
    }
    auto function = reachable->getFunction();
    auto functionUnderTest = functionsMap.find(function);
    if (functionUnderTest != functionsMap.end()) {
      functionsUnderTest.push_back((*functionUnderTest).second);
      functionsMap.erase(functionUnderTest);
    }
  }

  return functionsUnderTest;
}
} // namespace mull

mull::FunctionUnderTest::FunctionUnderTest(llvm::Function *function,
                                           mull::Test *test, int distance)
    : function(function) {
  addReachableTest(test, distance);
}

void mull::FunctionUnderTest::addReachableTest(mull::Test *test, int distance) {
  reachableTests.emplace_back(test, distance);
}

const std::vector<std::pair<mull::Test *, int>> &
mull::FunctionUnderTest::getReachableTests() const {
  return reachableTests;
}

llvm::Function *mull::FunctionUnderTest::getFunction() const {
  return function;
}

mull::ReachableFunction::ReachableFunction(llvm::Function *function,
                                           mull::Test *test, int distance)
    : function(function), test(test), distance(distance) {}

llvm::Function *mull::ReachableFunction::getFunction() const {
  return function;
}

mull::Test *mull::ReachableFunction::getTest() const { return test; }

int mull::ReachableFunction::getDistance() const { return distance; }
