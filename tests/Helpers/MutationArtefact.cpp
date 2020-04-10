#include "MutationArtefact.h"

#include "mull/Bitcode.h"

using namespace mull;
using namespace mull_test;

MutationArtefact::MutationArtefact(mull::SingleASTUnitMutations astMutations,
                                   std::vector<mull::MutationPoint *> nonJunkMutationPoints,
                                   std::vector<mull::MutationPoint *> junkMutationPoints,
                                   std::unique_ptr<llvm::LLVMContext> context,
                                   std::unique_ptr<mull::Bitcode> bitcode)
    : astMutations(astMutations), nonJunkMutationPoints(nonJunkMutationPoints),
      junkMutationPoints(junkMutationPoints), context(std::move(context)),
      bitcode(std::move(bitcode)) {}

const mull::SingleASTUnitMutations &MutationArtefact::getASTMutations() const {
  return astMutations;
}

const std::vector<mull::MutationPoint *> &MutationArtefact::getNonJunkMutationPoints() const {
  return nonJunkMutationPoints;
}

const std::vector<mull::MutationPoint *> &MutationArtefact::getJunkMutationPoints() const {
  return junkMutationPoints;
}
