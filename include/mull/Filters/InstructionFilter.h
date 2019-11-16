#pragma once

#include "mull/Filters/Filter.h"

#include <string>

namespace llvm {
class Instruction;
}

namespace mull {

class InstructionFilter : virtual public Filter {
public:
  virtual bool shouldSkip(llvm::Instruction *instruction) const = 0;
  virtual std::string name() = 0;
  virtual ~InstructionFilter() {};
};

} // namespace mull
