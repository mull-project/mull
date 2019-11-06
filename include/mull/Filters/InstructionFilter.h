#pragma once

#include <string>

namespace llvm {
class Instruction;
}

namespace mull {

class InstructionFilter {
public:
  virtual bool shouldSkip(llvm::Instruction *instruction) const = 0;
  virtual std::string name() = 0;
};

} // namespace mull
