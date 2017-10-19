#include "MutationOperatorsFactory.h"

#include "MutationOperator.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/MathDivMutationOperator.h"
#include "MutationOperators/MathMulMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "MutationOperators/ReplaceCallMutationOperator.h"
#include "MutationOperators/ScalarValueMutationOperator.h"
#include "Logger.h"

#include <llvm/ADT/STLExtras.h>

using namespace mull;

std::vector<std::unique_ptr<MutationOperator>>
MutationOperatorsFactory::mutationOperators(
  std::vector<std::string> mutationOperatorStrings) {

  if (mutationOperatorStrings.size() == 0) {
    Logger::info()
    << "Driver> No mutation operators specified in a config file. "
    << "Defaulting to default operators:" << "\n";

    auto mutationOperators = defaultMutationOperators();

    for (auto &mutationOperator: mutationOperators) {
      Logger::info() << "\t" << mutationOperator.get()->uniqueID() << "\n";
    }

    return mutationOperators;
  }

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  for (auto mutation : mutationOperatorStrings) {
    if (mutation == AddMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<AddMutationOperator>());
    }
    else if (mutation == AndOrReplacementMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());
    }
    else if (mutation == MathSubMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<MathSubMutationOperator>());
    }
    else if (mutation == MathMulMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<MathMulMutationOperator>());
    }
    else if (mutation == MathDivMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<MathDivMutationOperator>());
    }
    else if (mutation == NegateConditionMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());
    }
    else if (mutation == RemoveVoidFunctionMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());
    }
    else if (mutation == ReplaceCallMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<ReplaceCallMutationOperator>());
    }
    else if (mutation == ScalarValueMutationOperator::ID) {
      mutationOperators.emplace_back(make_unique<ScalarValueMutationOperator>());
    }
    else {
      Logger::info() << "Driver> Unknown Mutation Operator: " << mutation << "\n";
    }
  }

  if (mutationOperators.size() == 0) {
    Logger::info()
    << "Driver> No valid mutation operators found in a config file.\n";
  }

  return mutationOperators;
}

std::vector<std::unique_ptr<MutationOperator>>
MutationOperatorsFactory::defaultMutationOperators() {
  std::vector<std::unique_ptr<MutationOperator>> operators;

  operators.emplace_back(make_unique<AddMutationOperator>());
  operators.emplace_back(make_unique<NegateConditionMutationOperator>());
  operators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());

  return operators;
}
