#pragma once

#include "MutationOperator.h"

#include <string>
#include <vector>

namespace mull {

class MutationOperator;

class MutationOperatorsFactory {
  std::vector<std::unique_ptr<MutationOperator>> defaultMutationOperators();

public:

  std::vector<std::unique_ptr<MutationOperator>>
  mutationOperators(std::vector<std::string> mutationOperatorStrings);

};

} // namespace mull
