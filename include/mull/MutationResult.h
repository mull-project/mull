#pragma once

#include "mull/ExecutionResult.h"
#include "mull/Mutant.h"
#include <utility>

namespace mull {

class Mutant;

class MutationResult {
public:
  MutationResult(ExecutionResult result, Mutant *mutant)
      : result(std::move(result)), mutant(mutant) {}

  ExecutionResult &getExecutionResult() {
    return result;
  }
  Mutant *getMutant() {
    return mutant;
  }

private:
  ExecutionResult result;
  Mutant *mutant;
};
struct MutationResultComparator {
  bool operator()(std::unique_ptr<MutationResult> &lhs, std::unique_ptr<MutationResult> &rhs) {
    return operator()(*lhs, *rhs);
  }

  bool operator()(MutationResult &lhs, MutationResult &rhs) {
    MutantComparator cmp;
    return cmp(*lhs.getMutant(), *rhs.getMutant());
  }
};

} // namespace mull
