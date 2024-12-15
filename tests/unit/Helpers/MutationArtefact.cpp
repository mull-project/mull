#include "MutationArtefact.h"

#include <utility>

#include "mull/Bitcode.h"

using namespace mull;
using namespace mull_test;

MutationArtefact::MutationArtefact(std::vector<mull::MutationPoint *> nonJunkMutationPoints,
                                   std::vector<mull::MutationPoint *> junkMutationPoints,
                                   std::unique_ptr<llvm::LLVMContext> context,
                                   std::unique_ptr<mull::Bitcode> bitcode)
    : nonJunkMutationPoints(std::move(nonJunkMutationPoints)),
      junkMutationPoints(std::move(junkMutationPoints)), context(std::move(context)),
      bitcode(std::move(bitcode)) {}

const std::vector<mull::MutationPoint *> &MutationArtefact::getNonJunkMutationPoints() const {
  return nonJunkMutationPoints;
}

const std::vector<mull::MutationPoint *> &MutationArtefact::getJunkMutationPoints() const {
  return junkMutationPoints;
}
