#pragma once

#include "MutationOperators/MutationOperatorFilter.h"

namespace mull {

class GoogleTestMutationOperatorFilter : public MutationOperatorFilter {
public:
  bool shouldSkipInstruction(llvm::Instruction *instruction);
};

}
