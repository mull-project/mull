#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/LogicalAndToOr.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"
#include "mull/ReachableFunction.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/Mutators/CXX/NumberMutators.h>
#include <mull/Parallelization/Parallelization.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(MutationPoint, AndOrReplacementMutator_applyMutation) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_and_or_and_to_or_replacement_module_bc_path(), context, diagnostics);
  cxx::LogicalAndToOr mutator;

  FunctionUnderTest functionUnderTest(
      bitcode->getModule()->getFunction("testee_AND_operator_2branches"), nullptr, 0);
  auto mutationPoints = mutator.getMutations(bitcode.get(), functionUnderTest);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction =
      mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<BranchInst>(&mutatedInstruction));
}

TEST(MutationPoint, ScalarValueMutator_applyMutation) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_scalar_value_module_bc_path(), context, diagnostics);

  ScalarValueMutator mutator;
  FunctionUnderTest functionUnderTest(
      bitcode->getModule()->getFunction("scalar_value"), nullptr, 0);
  functionUnderTest.selectInstructions({});
  auto mutationPoints = mutator.getMutations(bitcode.get(), functionUnderTest);

  ASSERT_EQ(4U, mutationPoints.size());

  MutationPoint *mutationPoint1 = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint1->getAddress();
  ASSERT_TRUE(isa<StoreInst>(mutationPoint1->getOriginalValue()));

  MutationPoint *mutationPoint2 = mutationPoints[1];
  MutationPointAddress mutationPointAddress2 = mutationPoint2->getAddress();
  ASSERT_TRUE(isa<StoreInst>(mutationPoint2->getOriginalValue()));

  MutationPoint *mutationPoint3 = mutationPoints[2];
  MutationPointAddress mutationPointAddress3 = mutationPoint3->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint3->getOriginalValue()));

  MutationPoint *mutationPoint4 = mutationPoints[3];
  MutationPointAddress mutationPointAddress4 = mutationPoint4->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint4->getOriginalValue()));

  mutationPoint1->setMutatedFunction(mutationPoint1->getOriginalFunction());
  mutationPoint1->applyMutation();

  auto &mutatedInstruction =
      mutationPoint1->getAddress().findInstruction(mutationPoint1->getOriginalFunction());
  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

TEST(MutationPoint, ReplaceCallMutator_applyMutation) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_replace_call_module_bc_path(), context, diagnostics);

  ReplaceCallMutator mutator;
  FunctionUnderTest functionUnderTest(
      bitcode->getModule()->getFunction("replace_call"), nullptr, 0);
  functionUnderTest.selectInstructions({});
  auto mutationPoints = mutator.getMutations(bitcode.get(), functionUnderTest);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint->getAddress();
  ASSERT_TRUE(isa<CallInst>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction =
      mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
}

TEST(MutationPoint, OriginalValuePresent) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::mutators_replace_assignment_module_bc_path(), context, diagnostics);

  cxx::NumberAssignConst mutator;
  FunctionUnderTest functionUnderTest(
      bitcode->getModule()->getFunction("replace_assignment"), nullptr, 0);
  functionUnderTest.selectInstructions({});
  auto mutationPoints = mutator.getMutations(bitcode.get(), functionUnderTest);

  ASSERT_EQ(2U, mutationPoints.size());

  std::vector<std::string> mutantRepresentations;
  for (auto *mutation : mutationPoints) {
    std::string s;
    llvm::raw_string_ostream stream(s);
    mutation->getOriginalValue()->print(stream);
    mutantRepresentations.push_back(s);
  }

  for (auto *mutation : mutationPoints) {
    bitcode->addMutation(mutation);
  }

  std::vector<std::string> mutatedFunctions;
  CloneMutatedFunctionsTask::cloneFunctions(*bitcode, mutatedFunctions);
  DeleteOriginalFunctionsTask::deleteFunctions(*bitcode);
  InsertMutationTrampolinesTask::insertTrampolines(*bitcode);

  for (auto *mutation : mutationPoints) {
    mutation->applyMutation();
  }

  for (auto i = 0; i < mutationPoints.size(); i++) {
    auto mutation = mutationPoints[i];
    std::string s;
    llvm::raw_string_ostream stream(s);
    mutation->getOriginalValue()->print(stream);
    ASSERT_EQ(s, mutantRepresentations[i]);
  }
}
