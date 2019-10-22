//
// Created by Stanislav Pankevich on 2019-10-18.
//

#pragma once

#include <llvm/IR/Instruction.h>

namespace mull {

class InstructionFilter {
public:
  virtual bool validMutation(llvm::Instruction &instruction) const = 0;
  virtual bool validScalarMutation(llvm::Instruction &instruction) const = 0;
  virtual ~InstructionFilter() {}
};

class NullInstructionFilter : public InstructionFilter {
public:
  bool validMutation(llvm::Instruction &instruction) const override {
    return true;
  }
  bool validScalarMutation(llvm::Instruction &instruction) const override {
    return true;
  }
  ~NullInstructionFilter() {}
};

} // namespace mull
