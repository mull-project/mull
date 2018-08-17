#include "Testee.h"

namespace mull {
std::vector<MergedTestee> mergeTestees(std::vector<std::unique_ptr<Testee>> &testees) {
  std::map<llvm::Function *, MergedTestee> testeesMap;

  for (auto &testee : testees) {
    auto function = testee->getTesteeFunction();
    auto test = testee->getTest();
    auto distance = testee->getDistance();
    auto merged = testeesMap.find(function);
    if (merged != testeesMap.end()) {
      merged->second.addReachableTest(test, distance);
    } else {
      testeesMap.insert(std::make_pair(function, MergedTestee(function, test, distance)));
    }
  }

  std::vector<MergedTestee> mergedTestees;
  for (auto &testee: testees) {
    /// TODO: We could have just accumulated contents from the map,
    /// But it would break the order of mutants: some of our tests depend on it
    if (testeesMap.empty()) {
      break;
    }
    auto function = testee->getTesteeFunction();
    auto mergedTestee = testeesMap.find(function);
    if (mergedTestee != testeesMap.end()) {
      mergedTestees.push_back((*mergedTestee).second);
      testeesMap.erase(mergedTestee);
    }
  }

  return mergedTestees;
}
}

mull::MergedTestee::MergedTestee(llvm::Function *function, mull::Test *test, int distance)
    : function(function)
{
  addReachableTest(test, distance);
}

void mull::MergedTestee::addReachableTest(mull::Test *test, int distance) {
  reachableTests.emplace_back(test, distance);
}

const std::vector<std::pair<mull::Test *, int>> &mull::MergedTestee::getReachableTests() const {
  return reachableTests;
}

llvm::Function *mull::MergedTestee::getTesteeFunction() const {
  return function;
}

mull::Testee::Testee(llvm::Function *testeeFunction, mull::Test *test, int distance)
    : testeeFunction(testeeFunction), test(test), distance(distance) {}

llvm::Function *mull::Testee::getTesteeFunction() const {
  return testeeFunction;
}

mull::Test *mull::Testee::getTest() const {
  return test;
}

int mull::Testee::getDistance() const {
  return distance;
}
