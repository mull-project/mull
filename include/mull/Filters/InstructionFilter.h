#pragma once

#include "mull/Filters/Filter.h"

#include <string>

namespace llvm {
class Instruction;
}

namespace mull {

class InstructionFilter : virtual public Filter {
public:
  virtual bool shouldSkip(llvm::Instruction *instruction) = 0;
  virtual std::string name() = 0;
  ~InstructionFilter() override = default;
};

} // namespace mull
