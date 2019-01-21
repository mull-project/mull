#pragma once

#include "Mutator.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace mull {

class MutatorsFactory {
  std::map<std::string, std::unique_ptr<Mutator>> mutatorsMapping;
  std::map<std::string, std::vector<std::string>> groupsMapping;

  void init();
public:
  MutatorsFactory();
  std::vector<std::unique_ptr<Mutator>>
    mutators(const std::vector<std::string> &groups);
};

} // namespace mull
