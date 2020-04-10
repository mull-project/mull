#include "MutationArtefact.h"

#include "mull/Bitcode.h"

using namespace mull;
using namespace mull_test;

MutationArtefact::MutationArtefact(mull::SingleASTUnitMutations astMutations,
                                   std::vector<mull::MutationPoint *> nonJunkMutationPoints,
                                   std::vector<mull::MutationPoint *> junkMutationPoints,
                                   std::unique_ptr<mull::Bitcode> bitcode,
                                   std::unique_ptr<llvm::LLVMContext> context)
  : astMutations(astMutations), nonJunkMutationPoints(nonJunkMutationPoints),
    junkMutationPoints(junkMutationPoints), bitcode(std::move(bitcode)),
    context(std::move(context)) {}

MutationArtefact::~MutationArtefact() {
  /// We need to enforce the order of deallocation:
  /// First we deallocate the LLVM bitcode and then the underlying LLVM context.
  /// Doing this explicitly in order to avoid implicit deallocation of
  /// MutationArtefact's members.
  bitcode.reset();
  context.reset();
}

const mull::SingleASTUnitMutations &MutationArtefact::getASTMutations() const {
  return astMutations;
}

const std::vector<mull::MutationPoint *> &MutationArtefact::getNonJunkMutationPoints() const {
  return nonJunkMutationPoints;
}

const std::vector<mull::MutationPoint *> &MutationArtefact::getJunkMutationPoints() const {
  return junkMutationPoints;
}
