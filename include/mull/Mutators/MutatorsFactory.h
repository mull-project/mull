#pragma once

#include "Mutator.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mull {

class MutatorsFactory {
public:
  MutatorsFactory();
  std::vector<std::unique_ptr<Mutator>> mutators(const std::vector<std::string> &groups);
  void init();
  static std::string descriptionForGroup(const std::vector<std::string> &groupMembers);
  std::vector<std::pair<std::string, std::string>> commandLineOptions();
  std::map<std::string, std::unique_ptr<Mutator>> &getMutatorsMapping();
  std::map<std::string, std::vector<std::string>> &getGroupsMapping();

private:
  std::map<std::string, std::unique_ptr<Mutator>> mutatorsMapping;
  std::map<std::string, std::vector<std::string>> groupsMapping;
};

} // namespace mull
