#include "mull/Mutators/MutatorsFactory.h"

#include "mull/Logger.h"
#include "mull/Mutators/CXX/ArithmeticMutators.h"
#include "mull/Mutators/CXX/BitwiseMutators.h"
#include "mull/Mutators/CXX/LogicalAndToOr.h"
#include "mull/Mutators/CXX/LogicalOrToAnd.h"
#include "mull/Mutators/CXX/NumberMutators.h"
#include "mull/Mutators/CXX/RelationalMutators.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"
#include <llvm/ADT/STLExtras.h>
#include <set>
#include <sstream>

using namespace mull;
using namespace std;

static const string Experimental = "experimental";
static const string AllMutatorsGroup = "all";

static const string CXX_Assignment = "cxx_assignment";
static const string CXX_Const_Assignment = "cxx_const_assignment";
static const string CXX_Arithmetic_Assignment = "cxx_arithmetic_assignment";
static const string CXX_Bitwise_Assignment = "cxx_bitwise_assignment";
static const string CXX_Increment = "cxx_increment";
static const string CXX_Decrement = "cxx_decrement";
static const string CXX_Arithmetic = "cxx_arithmetic";
static const string CXX_Bitwise = "cxx_bitwise";
static const string CXX_Logical = "cxx_logical";
static const string CXX_Comparison = "cxx_comparison";
static const string CXX_Boundary = "cxx_boundary";
static const string CXX_All = "cxx_all";
static const string CXX_Default = "cxx_default";

static void expandGroups(const vector<string> &groups, const map<string, vector<string>> &mapping,
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
  groupsMapping[CXX_Const_Assignment] = {
    cxx::NumberAssignConst::ID, // a = b | a = Const
    cxx::NumberInitConst::ID,   // a(b)  | a(Const)
  };

  groupsMapping[CXX_Bitwise_Assignment] = {
    cxx::AndAssignToOrAssign::ID,        // &=  | |=
    cxx::OrAssignToAndAssign::ID,        // |=  | &=
    cxx::XorAssignToOrAssign::ID,        // ^=  | |=
    cxx::LShiftAssignToRShiftAssign::ID, // <<= | >>=
    cxx::RShiftAssignToLShiftAssign::ID, // >>= | <<=
  };

  groupsMapping[CXX_Arithmetic_Assignment] = {
    cxx::AddAssignToSubAssign::ID, // += | -=
    cxx::SubAssignToAddAssign::ID, // -= | +=
    cxx::MulAssignToDivAssign::ID, // *= | /=
    cxx::DivAssignToMulAssign::ID, // /= | *=
    cxx::RemAssignToDivAssign::ID, // %= | /=
  };

  groupsMapping[CXX_Increment] = {
    cxx::PreIncToPreDec::ID,   // ++x | --x
    cxx::PostIncToPostDec::ID, // x++ | x--
  };

  groupsMapping[CXX_Decrement] = {
    cxx::PreDecToPreInc::ID,   // --x | ++x
    cxx::PostDecToPostInc::ID, // x-- | x++
  };

  groupsMapping[CXX_Arithmetic] = {
    cxx::UnaryMinusToNoop::ID, // -x     | x
    cxx::AddToSub::ID,         // a + b  | a - b
    cxx::SubToAdd::ID,         // a - b  | a + b
    cxx::MulToDiv::ID,         // a * b  | a / b
    cxx::DivToMul::ID,         // a / b  | a * b
    cxx::RemToDiv::ID,         // a % b  | a / b
  };

  groupsMapping[CXX_Bitwise] = {
    cxx::BitwiseNotToNoop::ID, // ~x     | x
    cxx::BitwiseAndToOr::ID,   // a & b  | a | b
    cxx::BitwiseOrToAnd::ID,   // a | b  | a & b
    cxx::XorToOr::ID,          // a ^ b  |
    cxx::LShiftToRShift::ID,   // a << b | a >> b
    cxx::RShiftToLShift::ID,   // a >> b | a << b
  };

  groupsMapping[CXX_Logical] = {
    cxx::LogicalAndToOr::ID, // a && b | a || b
    cxx::LogicalOrToAnd::ID, // a || b | a && b
  };

  groupsMapping[CXX_Comparison] = {
    cxx::EqualToNotEqual::ID,          // == | !=
    cxx::NotEqualToEqual::ID,          // != | ==
    cxx::LessOrEqualToGreaterThan::ID, // <= | >
    cxx::LessThanToGreaterOrEqual::ID, // <  | >=
    cxx::GreaterOrEqualToLessThan::ID, // >= | <
    cxx::GreaterThanToLessOrEqual::ID, // >  | <=
  };

  groupsMapping[CXX_Boundary] = {
    cxx::LessOrEqualToLessThan::ID,       // <= | <
    cxx::LessThanToLessOrEqual::ID,       // <  | <=
    cxx::GreaterOrEqualToGreaterThan::ID, // >= | >
    cxx::GreaterThanToGreaterOrEqual::ID, // >  | >=
  };

  groupsMapping[CXX_Assignment] = {
    CXX_Bitwise_Assignment,
    CXX_Arithmetic_Assignment,
    CXX_Const_Assignment,
  };

  groupsMapping[CXX_All] = {
    CXX_Assignment, CXX_Increment, CXX_Decrement, CXX_Arithmetic, CXX_Comparison, CXX_Boundary,
  };

  groupsMapping[CXX_Default] = {
    CXX_Increment,
    CXX_Arithmetic,
    CXX_Comparison,
    CXX_Boundary,
  };

  groupsMapping[Experimental] = { NegateConditionMutator::ID,
                                  RemoveVoidFunctionMutator::ID,
                                  ScalarValueMutator::ID,
                                  ReplaceCallMutator::ID,
                                  CXX_Logical };
  groupsMapping[AllMutatorsGroup] = { CXX_All, Experimental };
}

template <typename MutatorClass>
void addMutator(std::map<std::string, std::unique_ptr<Mutator>> &mapping) {
  mapping[MutatorClass::ID] = make_unique<MutatorClass>();
}

void MutatorsFactory::init() {
  addMutator<NegateConditionMutator>(mutatorsMapping);
  addMutator<ReplaceCallMutator>(mutatorsMapping);
  addMutator<RemoveVoidFunctionMutator>(mutatorsMapping);
  addMutator<ScalarValueMutator>(mutatorsMapping);

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

  addMutator<cxx::LogicalAndToOr>(mutatorsMapping);
  addMutator<cxx::LogicalOrToAnd>(mutatorsMapping);

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

vector<unique_ptr<Mutator>> MutatorsFactory::mutators(const vector<string> &groups) {
  /// We need to recreate all mutators in case this method called
  /// more than once. It does not happen during normal program execution,
  /// but happens a lot during testing
  init();

  set<string> expandedGroups;

  if (groups.empty()) {
    expandGroups({ CXX_Default }, groupsMapping, expandedGroups);
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
    Logger::error() << "No valid mutators specified.\n";
  }

  return mutators;
}

/// Command Line Options

std::string MutatorsFactory::descriptionForGroup(const std::vector<std::string> &groupMembers) {
  if (groupMembers.empty()) {
    return std::string("empty group?");
  }

  std::stringstream members;
  std::copy(groupMembers.begin(),
            groupMembers.end() - 1,
            std::ostream_iterator<std::string>(members, ", "));
  members << *(groupMembers.end() - 1);

  return members.str();
}

std::vector<std::pair<std::string, std::string>> MutatorsFactory::commandLineOptions() {
  std::vector<std::pair<std::string, std::string>> options;
  for (auto &group : groupsMapping) {
    options.emplace_back(group.first, descriptionForGroup(group.second));
  }

  std::set<std::string> mutatorsSet;
  std::vector<std::string> groups({ AllMutatorsGroup });
  expandGroups({ AllMutatorsGroup }, groupsMapping, mutatorsSet);

  auto allMutators = mutators({ AllMutatorsGroup });

  for (auto &mutator : allMutators) {
    options.emplace_back(mutator->getUniqueIdentifier(), mutator->getDescription());
  }

  return options;
}

std::map<std::string, std::unique_ptr<Mutator>> &MutatorsFactory::getMutatorsMapping() {
  return mutatorsMapping;
}

std::map<std::string, std::vector<std::string>> &MutatorsFactory::getGroupsMapping() {
  return groupsMapping;
}
