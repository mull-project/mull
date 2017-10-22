#pragma once

#include "MutationOperator.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace mull {

class MutationOperatorsFactory {
  std::map<std::string, std::unique_ptr<MutationOperator>> mutationsMapping;
  std::map<std::string, std::vector<std::string>> groupsMapping;

  void init();
public:
  MutationOperatorsFactory();
  std::vector<std::unique_ptr<MutationOperator>>
    mutationOperators(const std::vector<std::string> groups);
};

} // namespace mull
