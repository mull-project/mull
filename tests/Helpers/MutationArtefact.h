#pragma once

#include <memory>
#include <vector>

namespace mull {
class MutationPoint;
class Bitcode;
} // namespace mull

namespace llvm {
class LLVMContext;
}

namespace mull_test {

class MutationArtefact {
public:
  MutationArtefact(std::vector<mull::MutationPoint *> nonJunkMutationPoints,
                   std::vector<mull::MutationPoint *> junkMutationPoints,
                   std::unique_ptr<llvm::LLVMContext> context,
                   std::unique_ptr<mull::Bitcode> bitcode);
  const std::vector<mull::MutationPoint *> &getNonJunkMutationPoints() const;
  const std::vector<mull::MutationPoint *> &getJunkMutationPoints() const;

private:
  std::vector<mull::MutationPoint *> nonJunkMutationPoints;
  std::vector<mull::MutationPoint *> junkMutationPoints;

  /// We need to enforce the order of deallocation:
  /// First we deallocate the LLVM bitcode and then the underlying LLVM context.
  /// Doing this explicitly in order to avoid implicit deallocation of
  /// MutationArtefact's members.
  /// C++ deallocates members in the reverse order, so bitcode goes first.
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<mull::Bitcode> bitcode;
};

} // namespace mull_test
