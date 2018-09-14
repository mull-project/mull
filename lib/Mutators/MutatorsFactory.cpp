#include "Mutators/MutatorsFactory.h"

#include "Mutators/Mutator.h"
#include "Mutators/AndOrReplacementMutator.h"
#include "Mutators/MathAddMutator.h"
#include "Mutators/MathDivMutator.h"
#include "Mutators/MathMulMutator.h"
#include "Mutators/MathSubMutator.h"
#include "Mutators/NegateConditionMutator.h"
#include "Mutators/RemoveVoidFunctionMutator.h"
#include "Mutators/ReplaceAssignmentMutator.h"
#include "Mutators/ReplaceCallMutator.h"
#include "Mutators/ScalarValueMutator.h"
#include "Mutators/ConditionalsBoundaryMutator.h"

#include "Logger.h"

#include <llvm/ADT/STLExtras.h>

#include <set>

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
  for (const string &group: groups) {
    if (mapping.count(group) == 0) {
      expandedGroups.insert(group);
      continue;
    }
    expandGroups(mapping.at(group), mapping, expandedGroups);
  }
}

MutatorsFactory::MutatorsFactory() {
  groupsMapping[ConditionalMutatorsGroup] = {
    AndOrReplacementMutator::ID,
    NegateConditionMutator::ID,
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[MathMutatorsGroup] = {
    MathAddMutator::ID,
    MathSubMutator::ID,
    MathMulMutator::ID,
    MathDivMutator::ID
  };
  groupsMapping[FunctionsMutatorsGroup] = {
    ReplaceCallMutator::ID,
    RemoveVoidFunctionMutator::ID
  };
  groupsMapping[ConstantMutatorsGroup] = {
    ScalarValueMutator::ID
  };
  groupsMapping[DefaultMutatorsGroup] = {
    MathAddMutator::ID,
    NegateConditionMutator::ID,
    RemoveVoidFunctionMutator::ID
  };
  groupsMapping[ExperimentalMutatorsGroup] = {
    MathSubMutator::ID,
    MathMulMutator::ID,
    MathDivMutator::ID,
    AndOrReplacementMutator::ID,
    ReplaceAssignmentMutator::ID,
    ReplaceCallMutator::ID,
    ScalarValueMutator::ID,
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[CXXMutatorsGroup] = {
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[AllMutatorsGroup] = {
    DefaultMutatorsGroup,
    ExperimentalMutatorsGroup
  };
}

void MutatorsFactory::init() {
  mutatorsMapping[MathAddMutator::ID] =
    make_unique<MathAddMutator>();
  mutatorsMapping[MathSubMutator::ID] =
    make_unique<MathSubMutator>();
  mutatorsMapping[MathDivMutator::ID] =
    make_unique<MathDivMutator>();
  mutatorsMapping[MathMulMutator::ID] =
    make_unique<MathMulMutator>();

  mutatorsMapping[NegateConditionMutator::ID] =
    make_unique<NegateConditionMutator>();
  mutatorsMapping[AndOrReplacementMutator::ID] =
    make_unique<AndOrReplacementMutator>();

  mutatorsMapping[ReplaceAssignmentMutator::ID] =
    make_unique<ReplaceAssignmentMutator>();
  mutatorsMapping[ReplaceCallMutator::ID] =
    make_unique<ReplaceCallMutator>();
  mutatorsMapping[RemoveVoidFunctionMutator::ID] =
    make_unique<RemoveVoidFunctionMutator>();

  mutatorsMapping[ScalarValueMutator::ID] =
    make_unique<ScalarValueMutator>();

  mutatorsMapping[ConditionalsBoundaryMutator::ID] =
    make_unique<ConditionalsBoundaryMutator>();
}

vector<unique_ptr<Mutator>>
MutatorsFactory::mutators(const vector<string> groups) {
  /// We need to recreate all mutators in case this method called
  /// more than once. It does not happen during normal program execution,
  /// but happens a lot during testing
  init();

  set<string> expandedGroups;

  if (groups.size() == 0) {
    Logger::info()
      << "No mutators specified in a config file.\n"
      << "Switching to default mutators.\n";

    expandGroups({ DefaultMutatorsGroup }, groupsMapping, expandedGroups);
  } else {
    expandGroups(groups, groupsMapping, expandedGroups);
  }

  vector<unique_ptr<Mutator>> mutators;

  for (string group: expandedGroups) {
    if (mutatorsMapping.count(group) == 0) {
      Logger::error() << "Unknown mutator: '" << group << "'\n";
      continue;
    }
    mutators.emplace_back(std::move(mutatorsMapping.at(group)));
    mutatorsMapping.erase(group);
  }

  if (mutators.size() == 0) {
    Logger::error()
      << "No valid mutators found in a config file.\n";
  }

  return mutators;
}
