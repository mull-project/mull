#pragma once

#include <llvm/IR/Function.h>

namespace mull {
class Test;

class MergedTestee {
public:
  explicit MergedTestee(llvm::Function *function, Test *test, int distance);
  void addReachableTest(Test *test, int distance);
  const std::vector<std::pair<Test *, int>> &getReachableTests() const;
  llvm::Function *getTesteeFunction() const;
private:
  std::vector<std::pair<Test *, int>> reachableTests;
  llvm::Function *function;
};

class Testee {
public:
  Testee(llvm::Function *testeeFunction, Test *test, int distance);
  llvm::Function *getTesteeFunction() const;
  Test *getTest() const;
  int getDistance() const;
private:
  llvm::Function *testeeFunction;
  Test *test;
  int distance;
};

std::vector<MergedTestee> mergeTestees(std::vector<std::unique_ptr<Testee>> &testees);

}
