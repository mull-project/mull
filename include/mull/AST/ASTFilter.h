#pragma once

#include "mull/AST/ASTMutations.h"
#include "mull/InstructionFilter.h"
#include "mull/MutationFilters/FilePathFilter.h"

namespace mull {

class ASTFilter : public InstructionFilter {
public:
  explicit ASTFilter(const ASTMutations &astMutations,
                     const FilePathFilter &filePathFilter);
  ~ASTFilter() {}

  bool validMutation(llvm::Instruction &instruction) const;
  bool validScalarMutation(llvm::Instruction &instruction) const;

private:
  const ASTMutations &astMutations;
  const FilePathFilter &filePathFilter;
};
} // namespace mull
