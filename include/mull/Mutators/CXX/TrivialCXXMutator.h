#pragma once

#include "mull/Mutators/Mutator.h"
#include <irm/irm.h>
#include <memory>
#include <mull/FunctionUnderTest.h>
#include <vector>

namespace mull {

class ReachableFunction;

namespace cxx {

class TrivialCXXMutator : public Mutator {
public:
  TrivialCXXMutator(std::vector<std::unique_ptr<irm::IRMutation>> mutators,
                    MutatorKind kind, std::string id, std::string description,
                    std::string replacement, std::string diagnostics);

  std::string getUniqueIdentifier() override;
  std::string getUniqueIdentifier() const override;
  std::string getDescription() const override;
  std::string getDiagnostics() const override;
  std::string getReplacement() const override;
  MutatorKind mutatorKind() override;

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *>
  getMutations(Bitcode *bitcode, const FunctionUnderTest &function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
  MutatorKind kind;

  std::string ID;
  std::string description;
  std::string replacement;
  std::string diagnostics;
};

} // namespace cxx

} // namespace mull