#include "mull/Mutators/MutatorsFactory.h"

#include "mull/Mutators/AndOrReplacementMutator.h"
#include "mull/Mutators/ConditionalsBoundaryMutator.h"
#include "mull/Mutators/MathAddMutator.h"
#include "mull/Mutators/MathDivMutator.h"
#include "mull/Mutators/MathMulMutator.h"
#include "mull/Mutators/MathSubMutator.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/Mutators/ReplaceAssignmentMutator.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"

#include "mull/Logger.h"

#include <llvm/ADT/STLExtras.h>

#include <set>
#include <sstream>

using namespace mull;
using namespace std;

static const string MathMutatorsGroup = "math";
static const string ConditionalMutatorsGroup = "conditional";
static const string FunctionsMutatorsGroup = "functions";
static const string ConstantMutatorsGroup = "constant";

static const string DefaultMutatorsGroup = "default";
static const string ExperimentalMutatorsGroup = "experimental";

static const string CXXMutatorsGroup = "cxx";

static const string AllMutatorsGroup = "all";

static void expandGroups(const vector<string> &groups,
                         const map<string, vector<string>> &mapping,
                         set<string> &expandedGroups) {
  for (const string &group : groups) {
    if (mapping.count(group) == 0) {
      expandedGroups.insert(group);
      continue;
    }
    expandGroups(mapping.at(group), mapping, expandedGroups);
  }
}

MutatorsFactory::MutatorsFactory() {
  groupsMapping[ConditionalMutatorsGroup] = {AndOrReplacementMutator::ID,
                                             NegateConditionMutator::ID,
                                             ConditionalsBoundaryMutator::ID};
  groupsMapping[MathMutatorsGroup] = {MathAddMutator::ID, MathSubMutator::ID,
                                      MathMulMutator::ID, MathDivMutator::ID};
  groupsMapping[FunctionsMutatorsGroup] = {ReplaceCallMutator::ID,
                                           RemoveVoidFunctionMutator::ID};
  groupsMapping[ConstantMutatorsGroup] = {ScalarValueMutator::ID};
  groupsMapping[DefaultMutatorsGroup] = {MathAddMutator::ID,
                                         NegateConditionMutator::ID,
                                         RemoveVoidFunctionMutator::ID};
  groupsMapping[ExperimentalMutatorsGroup] = {
      MathSubMutator::ID,           MathMulMutator::ID,
      MathDivMutator::ID,           AndOrReplacementMutator::ID,
      ReplaceAssignmentMutator::ID, ReplaceCallMutator::ID,
      ScalarValueMutator::ID,       ConditionalsBoundaryMutator::ID};
  groupsMapping[CXXMutatorsGroup] = {ConditionalsBoundaryMutator::ID};
  groupsMapping[AllMutatorsGroup] = {DefaultMutatorsGroup,
                                     ExperimentalMutatorsGroup};
}

void MutatorsFactory::init() {
  mutatorsMapping[MathAddMutator::ID] = make_unique<MathAddMutator>();
  mutatorsMapping[MathSubMutator::ID] = make_unique<MathSubMutator>();
  mutatorsMapping[MathDivMutator::ID] = make_unique<MathDivMutator>();
  mutatorsMapping[MathMulMutator::ID] = make_unique<MathMulMutator>();

  mutatorsMapping[NegateConditionMutator::ID] =
      make_unique<NegateConditionMutator>();
  mutatorsMapping[AndOrReplacementMutator::ID] =
      make_unique<AndOrReplacementMutator>();

  mutatorsMapping[ReplaceAssignmentMutator::ID] =
      make_unique<ReplaceAssignmentMutator>();
  mutatorsMapping[ReplaceCallMutator::ID] = make_unique<ReplaceCallMutator>();
  mutatorsMapping[RemoveVoidFunctionMutator::ID] =
      make_unique<RemoveVoidFunctionMutator>();

  mutatorsMapping[ScalarValueMutator::ID] = make_unique<ScalarValueMutator>();

  mutatorsMapping[ConditionalsBoundaryMutator::ID] =
      make_unique<ConditionalsBoundaryMutator>();
}

vector<unique_ptr<Mutator>>
MutatorsFactory::mutators(const vector<string> &groups) {
  /// We need to recreate all mutators in case this method called
  /// more than once. It does not happen during normal program execution,
  /// but happens a lot during testing
  init();

  set<string> expandedGroups;

  if (groups.empty()) {
    expandGroups({DefaultMutatorsGroup}, groupsMapping, expandedGroups);
  } else {
    expandGroups(groups, groupsMapping, expandedGroups);
  }

  vector<unique_ptr<Mutator>> mutators;

  for (const string &group : expandedGroups) {
    if (mutatorsMapping.count(group) == 0) {
      Logger::error() << "Unknown mutator: '" << group << "'\n";
      continue;
    }
    mutators.emplace_back(std::move(mutatorsMapping.at(group)));
    mutatorsMapping.erase(group);
  }

  if (mutators.empty()) {
    Logger::error() << "No valid mutators found in a config file.\n";
  }

  return mutators;
}

/// Command Line Options

std::string descriptionForGroup(const std::vector<std::string> &groupMembers) {
  if (groupMembers.empty()) {
    return std::string("empty group?");
  }

  std::stringstream members;
  std::copy(groupMembers.begin(), groupMembers.end() - 1,
            std::ostream_iterator<std::string>(members, ", "));
  members << *(groupMembers.end() - 1);

  return members.str();
}

#include <iostream>

std::vector<std::pair<std::string, std::string>>
MutatorsFactory::commandLineOptions() {
  std::vector<std::pair<std::string, std::string>> options;
  for (auto &group : groupsMapping) {
    options.emplace_back(group.first, descriptionForGroup(group.second));
  }

  std::set<std::string> mutatorsSet;
  std::vector<std::string> groups({AllMutatorsGroup});
  expandGroups({AllMutatorsGroup}, groupsMapping, mutatorsSet);

  auto allMutators = mutators({AllMutatorsGroup});

  for (auto &mutator : allMutators) {
    options.emplace_back(mutator->getUniqueIdentifier(),
                         mutator->getDescription());
  }

  return options;
}
