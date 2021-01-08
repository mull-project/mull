#pragma once

#include "mull/ExecutionResult.h"
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

} // namespace mull
