#pragma once

#include "Mutant.h"
#include "MutationPoint.h"
#include "MutationResult.h"
#include <vector>

namespace mull {

class Result {
public:
  Result(std::vector<std::unique_ptr<Mutant>> mutants,
         std::vector<std::unique_ptr<MutationResult>> mutationResults,
         std::vector<MutationPoint *> mutationPoints)
      : mutants(std::move(mutants)), mutationResults(std::move(mutationResults)),
        mutationPoints(std::move(mutationPoints)) {}

  std::vector<std::unique_ptr<Mutant>> const &getMutants() const {
    return mutants;
  }

  std::vector<std::unique_ptr<MutationResult>> const &getMutationResults() const {
    return mutationResults;
  }

  std::vector<MutationPoint *> const &getMutationPoints() const {
    return mutationPoints;
  }

private:
  std::vector<std::unique_ptr<Mutant>> mutants;
  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  std::vector<MutationPoint *> mutationPoints;
};
} // namespace mull
