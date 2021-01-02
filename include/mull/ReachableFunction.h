#pragma once

#include <vector>

namespace llvm {
class Function;
class Instruction;
} // namespace llvm

namespace mull {
class InstructionFilter;

class FunctionUnderTest {
public:
  explicit FunctionUnderTest(llvm::Function *function);
  llvm::Function *getFunction() const;
  const std::vector<llvm::Instruction *> &getSelectedInstructions() const;
  void selectInstructions(const std::vector<InstructionFilter *> &filters);

private:
  llvm::Function *function;
  std::vector<llvm::Instruction *> selectedInstructions;
};

} // namespace mull
