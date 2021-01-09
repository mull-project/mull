#pragma once

#include <vector>

namespace llvm {
class Function;
class Instruction;
} // namespace llvm

namespace mull {
class InstructionFilter;
class Bitcode;

class FunctionUnderTest {
public:
  FunctionUnderTest(llvm::Function *function, Bitcode *bitcode, bool covered = true);
  llvm::Function *getFunction() const;
  Bitcode *getBitcode() const;
  const std::vector<llvm::Instruction *> &getSelectedInstructions() const;
  bool isCovered() const;
  void selectInstructions(const std::vector<InstructionFilter *> &filters);

private:
  llvm::Function *function;
  Bitcode *bitcode;
  bool covered;
  std::vector<llvm::Instruction *> selectedInstructions;
};

} // namespace mull
