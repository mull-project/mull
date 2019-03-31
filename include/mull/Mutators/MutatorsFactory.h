#pragma once

#include "Mutator.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mull {

class MutatorsFactory {
  std::map<std::string, std::unique_ptr<Mutator>> mutatorsMapping;
  std::map<std::string, std::vector<std::string>> groupsMapping;

  void init();

public:
  MutatorsFactory();
  std::vector<std::unique_ptr<Mutator>>
  mutators(const std::vector<std::string> &groups);

  std::vector<std::pair<std::string, std::string>> commandLineOptions();
};

} // namespace mull
