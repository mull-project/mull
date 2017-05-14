#pragma once

#include "MutationOperators/MutationOperatorFilter.h"

namespace llvm {
  class Instruction;
  class Function;
}

namespace mull {

class GoogleTestMutationOperatorFilter : public MutationOperatorFilter {
public:
  bool shouldSkipDefinedFunction(llvm::Function *definedFunction);
  bool shouldSkipInstruction(llvm::Instruction *instruction);
};

}
