#include "mull/Mutators/MutatorsFactory.h"

#include "mull/Mutators/CXX/ArithmeticMutators.h"
#include "mull/Mutators/CXX/BitwiseMutators.h"
#include "mull/Mutators/CXX/CallMutators.h"
#include "mull/Mutators/CXX/NumberMutators.h"
#include "mull/Mutators/CXX/RelationalMutators.h"
#include "mull/Mutators/CXX/RemoveNegation.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "rust/mull-cxx-bridge/bridge.rs.h"
#include <llvm/ADT/STLExtras.h>
#include <unordered_set>

using namespace mull;
using namespace std;

MutatorsFactory::MutatorsFactory(const MullDiagnostics &diagnostics) : diagnostics(diagnostics) {}

template <typename MutatorClass>
void addMutator(std::map<std::string, std::unique_ptr<Mutator>> &mapping) {
  mapping[MutatorClass::ID()] = std::make_unique<MutatorClass>();
}

void MutatorsFactory::init() {
  addMutator<NegateConditionMutator>(mutatorsMapping);

  addMutator<cxx::RemoveVoidCall>(mutatorsMapping);
  addMutator<cxx::ReplaceScalarCall>(mutatorsMapping);

  addMutator<cxx::NumberAssignConst>(mutatorsMapping);
  addMutator<cxx::NumberInitConst>(mutatorsMapping);
  addMutator<cxx::AddAssignToSubAssign>(mutatorsMapping);
  addMutator<cxx::SubAssignToAddAssign>(mutatorsMapping);
  addMutator<cxx::MulAssignToDivAssign>(mutatorsMapping);
  addMutator<cxx::DivAssignToMulAssign>(mutatorsMapping);
  addMutator<cxx::RemAssignToDivAssign>(mutatorsMapping);
  addMutator<cxx::AndAssignToOrAssign>(mutatorsMapping);
  addMutator<cxx::OrAssignToAndAssign>(mutatorsMapping);
  addMutator<cxx::XorAssignToOrAssign>(mutatorsMapping);
  addMutator<cxx::LShiftAssignToRShiftAssign>(mutatorsMapping);
  addMutator<cxx::RShiftAssignToLShiftAssign>(mutatorsMapping);

  addMutator<cxx::PreIncToPreDec>(mutatorsMapping);
  addMutator<cxx::PostIncToPostDec>(mutatorsMapping);

  addMutator<cxx::PreDecToPreInc>(mutatorsMapping);
  addMutator<cxx::PostDecToPostInc>(mutatorsMapping);

  addMutator<cxx::UnaryMinusToNoop>(mutatorsMapping);
  addMutator<cxx::AddToSub>(mutatorsMapping);
  addMutator<cxx::SubToAdd>(mutatorsMapping);
  addMutator<cxx::MulToDiv>(mutatorsMapping);
  addMutator<cxx::DivToMul>(mutatorsMapping);
  addMutator<cxx::RemToDiv>(mutatorsMapping);
  addMutator<cxx::BitwiseNotToNoop>(mutatorsMapping);
  addMutator<cxx::BitwiseAndToOr>(mutatorsMapping);
  addMutator<cxx::BitwiseOrToAnd>(mutatorsMapping);
  addMutator<cxx::XorToOr>(mutatorsMapping);
  addMutator<cxx::LShiftToRShift>(mutatorsMapping);
  addMutator<cxx::RShiftToLShift>(mutatorsMapping);

  addMutator<cxx::RemoveNegation>(mutatorsMapping);

  addMutator<cxx::EqualToNotEqual>(mutatorsMapping);
  addMutator<cxx::NotEqualToEqual>(mutatorsMapping);
  addMutator<cxx::LessOrEqualToGreaterThan>(mutatorsMapping);
  addMutator<cxx::LessThanToGreaterOrEqual>(mutatorsMapping);
  addMutator<cxx::GreaterOrEqualToLessThan>(mutatorsMapping);
  addMutator<cxx::GreaterThanToLessOrEqual>(mutatorsMapping);

  addMutator<cxx::LessOrEqualToLessThan>(mutatorsMapping);
  addMutator<cxx::LessThanToLessOrEqual>(mutatorsMapping);
  addMutator<cxx::GreaterOrEqualToGreaterThan>(mutatorsMapping);
  addMutator<cxx::GreaterThanToGreaterOrEqual>(mutatorsMapping);
}

Mutator *MutatorsFactory::getMutator(const string &mutatorId) {
  return mutatorsMapping[mutatorId].get();
}

std::unordered_set<std::string>
MutatorsFactory::expandMutatorGroups(const std::vector<std::string> &groups) {
  // Use Rust as the source of truth for mutator group expansion
  rust::Vec<rust::String> rustGroups;
  for (const auto &group : groups) {
    std::string tempGroup = group;
    if (tempGroup == "all") {
      diagnostics.warning(
          "Group 'all' is replaced with 'cxx_all' and will be removed in a future release.");
      tempGroup = "cxx_all";
    }
    rustGroups.push_back(rust::String(tempGroup));
  }
  rust::Vec<rust::String> expanded = expand_mutator_groups(rustGroups);
  std::unordered_set<std::string> result;
  for (const auto &mutator : expanded) {
    result.insert(std::string(mutator));
  }
  return result;
}

vector<unique_ptr<Mutator>>
MutatorsFactory::mutators(const vector<string> &groups,
                          const std::vector<std::string> &ignoreGroups) {
  /// We need to recreate all mutators in case this method called
  /// more than once. It does not happen during normal program execution,
  /// but happens a lot during testing
  init();

  std::unordered_set<std::string> expandedGroups;
  std::unordered_set<std::string> expandedIgnoreGroups;
  if (!ignoreGroups.empty()) {
    expandedIgnoreGroups = expandMutatorGroups(ignoreGroups);
  }

  if (groups.empty()) {
    expandedGroups = expandMutatorGroups({ "cxx_default" });
  } else {
    expandedGroups = expandMutatorGroups(groups);
  }

  for (auto &ignoreGroup : expandedIgnoreGroups) {
    expandedGroups.erase(ignoreGroup);
  }

  std::vector<std::unique_ptr<Mutator>> mutators;
  std::vector<std::string> sortedGroups;
  std::copy(expandedGroups.begin(), expandedGroups.end(), std::back_inserter(sortedGroups));
  std::sort(sortedGroups.begin(), sortedGroups.end());

  for (const std::string &group : sortedGroups) {
    if (mutatorsMapping.count(group) == 0) {
      diagnostics.warning(std::string("Unknown mutator: ") + group);
      continue;
    }
    mutators.emplace_back(std::move(mutatorsMapping.at(group)));
    mutatorsMapping.erase(group);
  }

  if (mutators.empty()) {
    diagnostics.warning("No valid mutators specified.");
  }

  return mutators;
}

/// Command Line Options

std::vector<std::pair<std::string, std::string>> MutatorsFactory::commandLineOptions() {
  std::vector<std::pair<std::string, std::string>> options;

  // Get group definitions from Rust
  rust::Vec<MutatorGroupDef> groupDefs = get_mutator_group_definitions();
  for (const auto &def : groupDefs) {
    options.emplace_back(std::string(def.name), std::string(def.description));
  }

  // Get individual mutator descriptions
  auto allMutators = mutators({ "cxx_all" }, {});
  for (auto &mutator : allMutators) {
    options.emplace_back(mutator->getUniqueIdentifier(), mutator->getDescription());
  }

  return options;
}

std::map<std::string, std::unique_ptr<Mutator>> &MutatorsFactory::getMutatorsMapping() {
  return mutatorsMapping;
}
