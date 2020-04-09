#pragma once

#include "mull/AST/ASTMutationStorage.h"

#include <memory>
#include <vector>

namespace mull {
class MutationPoint;
class Bitcode;
}

namespace llvm {
class LLVMContext;
}

namespace mull_test {

class MutationArtefact {
public:
  MutationArtefact(mull::SingleASTUnitMutations astMutations,
                   std::vector<mull::MutationPoint *> nonJunkMutationPoints,
                   std::vector<mull::MutationPoint *> junkMutationPoints,
                   std::unique_ptr <mull::Bitcode> bitcode,
                   std::unique_ptr <llvm::LLVMContext> context);
  ~MutationArtefact();
  const mull::SingleASTUnitMutations &getASTMutations() const;
  const std::vector<mull::MutationPoint *> &getNonJunkMutationPoints() const;
  const std::vector<mull::MutationPoint *> &getJunkMutationPoints() const;
private:
  mull::SingleASTUnitMutations astMutations;
  std::vector<mull::MutationPoint *> nonJunkMutationPoints;
  std::vector<mull::MutationPoint *> junkMutationPoints;
  std::unique_ptr <mull::Bitcode> bitcode;
  std::unique_ptr <llvm::LLVMContext> context;
};

}
