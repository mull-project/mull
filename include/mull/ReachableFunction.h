#pragma once

#include <llvm/IR/Function.h>

namespace mull {
class Test;

class FunctionUnderTest {
public:
  FunctionUnderTest(llvm::Function *function, Test *test, int distance);
  void addReachableTest(Test *test, int distance);
  const std::vector<std::pair<Test *, int>> &getReachableTests() const;
  llvm::Function *getFunction() const;

private:
  std::vector<std::pair<Test *, int>> reachableTests;
  llvm::Function *function;
};

class ReachableFunction {
public:
  ReachableFunction(llvm::Function *function, Test *test, int distance);
  llvm::Function *getFunction() const;
  Test *getTest() const;
  int getDistance() const;

private:
  llvm::Function *function;
  Test *test;
  int distance;
};

std::vector<FunctionUnderTest> mergeReachableFunctions(
    std::vector<std::unique_ptr<ReachableFunction>> &functions);

} // namespace mull
