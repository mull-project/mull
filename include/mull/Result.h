#pragma once

#include "Mutant.h"
#include "MutationResult.h"
#include <vector>

namespace mull {

class Result {
public:
  Result(std::vector<std::unique_ptr<Mutant>> mutants,
         std::vector<std::unique_ptr<MutationResult>> mutationResults)
      : mutants(std::move(mutants)), mutationResults(std::move(mutationResults)) {}

  std::vector<std::unique_ptr<Mutant>> const &getMutants() const {
    return mutants;
  }

  std::vector<std::unique_ptr<MutationResult>> const &getMutationResults() const {
    return mutationResults;
  }

private:
  std::vector<std::unique_ptr<Mutant>> mutants;
  std::vector<std::unique_ptr<MutationResult>> mutationResults;
};
} // namespace mull
