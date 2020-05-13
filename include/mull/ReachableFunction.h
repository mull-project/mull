#pragma once

#include <llvm/IR/Function.h>

namespace mull {
class Test;
class InstructionFilter;

class FunctionUnderTest {
public:
  FunctionUnderTest(llvm::Function *function, Test *test, int distance);
  void addReachableTest(Test *test, int distance);
  const std::vector<std::pair<Test *, int>> &getReachableTests() const;
  llvm::Function *getFunction() const;
  const std::vector<llvm::Instruction *> &getSelectedInstructions() const;
  void selectInstructions(const std::vector<InstructionFilter *> &filters);

private:
  std::vector<std::pair<Test *, int>> reachableTests;
  llvm::Function *function;
  std::vector<llvm::Instruction *> selectedInstructions;
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

std::vector<FunctionUnderTest>
mergeReachableFunctions(std::vector<std::unique_ptr<ReachableFunction>> &functions);

} // namespace mull
