#include "MutationOperatorsFactory.h"

#include "MutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/MathAddMutationOperator.h"
#include "MutationOperators/MathDivMutationOperator.h"
#include "MutationOperators/MathMulMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "MutationOperators/ReplaceAssignmentMutationOperator.h"
#include "MutationOperators/ReplaceCallMutationOperator.h"
#include "MutationOperators/ScalarValueMutationOperator.h"

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

MutationOperatorsFactory::MutationOperatorsFactory() {
  groupsMapping[ConditionalOperatorsGroup] = {
    AndOrReplacementMutationOperator::ID,
    NegateConditionMutationOperator::ID,
    ConditionalsBoundaryMutator::ID
  };
  groupsMapping[MathOperatorsGroup] = {
    MathAddMutationOperator::ID,
    MathSubMutationOperator::ID,
    MathMulMutationOperator::ID,
    MathDivMutationOperator::ID
  };
  groupsMapping[FunctionsOperatorsGroup] = {
    ReplaceCallMutationOperator::ID,
    RemoveVoidFunctionMutationOperator::ID
  };
  groupsMapping[ConstantOperatorsGroup] = {
    ScalarValueMutationOperator::ID
  };
  groupsMapping[DefaultOperatorsGroup] = {
    MathAddMutationOperator::ID,
    NegateConditionMutationOperator::ID,
    RemoveVoidFunctionMutationOperator::ID
  };
  groupsMapping[ExperimentalOperatorsGroup] = {
    MathSubMutationOperator::ID,
    MathMulMutationOperator::ID,
    MathDivMutationOperator::ID,
    AndOrReplacementMutationOperator::ID,
    ReplaceAssignmentMutationOperator::ID,
    ReplaceCallMutationOperator::ID,
    ScalarValueMutationOperator::ID,
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

void MutationOperatorsFactory::init() {
  mutationsMapping[MathAddMutationOperator::ID] =
    make_unique<MathAddMutationOperator>();
  mutationsMapping[MathSubMutationOperator::ID] =
    make_unique<MathSubMutationOperator>();
  mutationsMapping[MathDivMutationOperator::ID] =
    make_unique<MathDivMutationOperator>();
  mutationsMapping[MathMulMutationOperator::ID] =
    make_unique<MathMulMutationOperator>();

  mutationsMapping[NegateConditionMutationOperator::ID] =
    make_unique<NegateConditionMutationOperator>();
  mutationsMapping[AndOrReplacementMutationOperator::ID] =
    make_unique<AndOrReplacementMutationOperator>();

  mutationsMapping[ReplaceAssignmentMutationOperator::ID] =
    make_unique<ReplaceAssignmentMutationOperator>();
  mutationsMapping[ReplaceCallMutationOperator::ID] =
    make_unique<ReplaceCallMutationOperator>();
  mutationsMapping[RemoveVoidFunctionMutationOperator::ID] =
    make_unique<RemoveVoidFunctionMutationOperator>();

  mutationsMapping[ScalarValueMutationOperator::ID] =
    make_unique<ScalarValueMutationOperator>();

  mutationsMapping[ConditionalsBoundaryMutator::ID] =
    make_unique<ConditionalsBoundaryMutator>();
}

vector<unique_ptr<MutationOperator>>
MutationOperatorsFactory::mutationOperators(const vector<string> groups) {
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

  vector<unique_ptr<MutationOperator>> mutationOperators;

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
