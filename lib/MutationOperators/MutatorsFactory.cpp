#include "MutatorsFactory.h"

#include "Mutator.h"
#include "MutationOperators/AndOrReplacementMutator.h"
#include "MutationOperators/MathAddMutator.h"
#include "MutationOperators/MathDivMutator.h"
#include "MutationOperators/MathMulMutator.h"
#include "MutationOperators/MathSubMutator.h"
#include "MutationOperators/NegateConditionMutator.h"
#include "MutationOperators/RemoveVoidFunctionMutator.h"
#include "MutationOperators/ReplaceAssignmentMutator.h"
#include "MutationOperators/ReplaceCallMutator.h"
#include "MutationOperators/ScalarValueMutator.h"

#include "MutationOperators/ConditionalsBoundaryMutator.h"

#include "Logger.h"

#include <llvm/ADT/STLExtras.h>

#include <set>

using namespace mull;
using namespace std;

static const string MathOperatorsGroup = "math";
static const string ConditionalOperatorsGroup = "conditional";
static const string FunctionsOperatorsGroup = "functions";
static const string ConstantOperatorsGroup = "constant";

static const string DefaultOperatorsGroup = "default";
static const string ExperimentalOperatorsGroup = "experimental";

static const string CXXOperatorsGroup = "cxx";

static const string AllOperatorsGroup = "all";

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
  groupsMapping[ConditionalOperatorsGroup] = {
    AndOrReplacementMutator::ID,
    NegateConditionMutator::ID,
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[MathOperatorsGroup] = {
    MathAddMutator::ID,
    MathSubMutator::ID,
    MathMulMutator::ID,
    MathDivMutator::ID
  };
  groupsMapping[FunctionsOperatorsGroup] = {
    ReplaceCallMutator::ID,
    RemoveVoidFunctionMutator::ID
  };
  groupsMapping[ConstantOperatorsGroup] = {
    ScalarValueMutator::ID
  };
  groupsMapping[DefaultOperatorsGroup] = {
    MathAddMutator::ID,
    NegateConditionMutator::ID,
    RemoveVoidFunctionMutator::ID
  };
  groupsMapping[ExperimentalOperatorsGroup] = {
    MathSubMutator::ID,
    MathMulMutator::ID,
    MathDivMutator::ID,
    AndOrReplacementMutator::ID,
    ReplaceAssignmentMutator::ID,
    ReplaceCallMutator::ID,
    ScalarValueMutator::ID,
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[CXXOperatorsGroup] = {
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[AllOperatorsGroup] = {
    DefaultOperatorsGroup,
    ExperimentalOperatorsGroup
  };
}

void MutatorsFactory::init() {
  mutationsMapping[MathAddMutator::ID] =
    make_unique<MathAddMutator>();
  mutationsMapping[MathSubMutator::ID] =
    make_unique<MathSubMutator>();
  mutationsMapping[MathDivMutator::ID] =
    make_unique<MathDivMutator>();
  mutationsMapping[MathMulMutator::ID] =
    make_unique<MathMulMutator>();

  mutationsMapping[NegateConditionMutator::ID] =
    make_unique<NegateConditionMutator>();
  mutationsMapping[AndOrReplacementMutator::ID] =
    make_unique<AndOrReplacementMutator>();

  mutationsMapping[ReplaceAssignmentMutator::ID] =
    make_unique<ReplaceAssignmentMutator>();
  mutationsMapping[ReplaceCallMutator::ID] =
    make_unique<ReplaceCallMutator>();
  mutationsMapping[RemoveVoidFunctionMutator::ID] =
    make_unique<RemoveVoidFunctionMutator>();

  mutationsMapping[ScalarValueMutator::ID] =
    make_unique<ScalarValueMutator>();

  mutationsMapping[ConditionalsBoundaryMutator::ID] =
    make_unique<ConditionalsBoundaryMutator>();
}

vector<unique_ptr<Mutator>>
MutatorsFactory::mutationOperators(const vector<string> groups) {
  /// We need to recreate all mutation operators in case this method called
  /// more than once. It does not happen during normal program execution,
  /// but happens a lot during testing
  init();

  set<string> expandedGroups;

  if (groups.size() == 0) {
    Logger::info()
      << "No mutation operators specified in a config file.\n"
      << "Switching to default operators.\n";

    expandGroups({ DefaultOperatorsGroup }, groupsMapping, expandedGroups);
  } else {
    expandGroups(groups, groupsMapping, expandedGroups);
  }

  vector<unique_ptr<Mutator>> mutationOperators;

  for (string group: expandedGroups) {
    if (mutationsMapping.count(group) == 0) {
      Logger::error() << "Unknown mutation operator: '" << group << "'\n";
      continue;
    }
    mutationOperators.emplace_back(std::move(mutationsMapping.at(group)));
    mutationsMapping.erase(group);
  }

  if (mutationOperators.size() == 0) {
    Logger::error()
      << "No valid mutation operators found in a config file.\n";
  }

  return mutationOperators;
}
