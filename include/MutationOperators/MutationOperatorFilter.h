#pragma once

namespace llvm {
  class Instruction;
}

namespace mull {

  class MutationOperatorFilter {
  public:
    virtual bool shouldSkipInstruction(llvm::Instruction *instruction) = 0;
    virtual ~MutationOperatorFilter() {}
  };

  class NullMutationOperatorFilter : public MutationOperatorFilter {
  public:
    bool shouldSkipInstruction(llvm::Instruction *instruction) { return false; };
  };

}
