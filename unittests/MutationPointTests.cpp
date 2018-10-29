#include "Context.h"
#include "Mutators/MathAddMutator.h"
#include "Mutators/MathDivMutator.h"
#include "Mutators/MathMulMutator.h"
#include "Mutators/MathSubMutator.h"
#include "Mutators/NegateConditionMutator.h"
#include "Mutators/AndOrReplacementMutator.h"
#include "Mutators/ScalarValueMutator.h"
#include "Mutators/ReplaceAssignmentMutator.h"
#include "Mutators/ReplaceCallMutator.h"
#include "TestModuleFactory.h"
#include "Testee.h"
#include "MutationsFinder.h"
#include "Filter.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(MutationPoint, SimpleTest_AddOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_CountLetters_Module();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = Ctx.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  Filter filter;
  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx,
                                                                         mergedTestees,
                                                                         filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  MutationPointAddress address = mutationPoint->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Sub, mutatedInstruction.getOpcode());
}

TEST(MutationPoint, SimpleTest_MathSubOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathSub_Module();

  Context context;
  context.addModule(std::move(module));

  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathSubMutator>());
  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("math_sub");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context, mergedTestees, filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  MutationPointAddress address = mutationPoint->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Add, mutatedInstruction.getOpcode());
}

TEST(MutationPoint, SimpleTest_MathMulOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathMul_Module();

  Context context;
  context.addModule(std::move(module));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathMulMutator>());
  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("math_mul");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context, mergedTestees, filter);

  ASSERT_EQ(mutationPoints.size(), 1UL);

  MutationPoint *mutationPoint = mutationPoints.front();
  MutationPointAddress address = mutationPoint->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::SDiv, mutatedInstruction.getOpcode());
}

TEST(MutationPoint, SimpleTest_MathDivOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathDiv_Module();

  Context context;
  context.addModule(std::move(module));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathDivMutator>());

  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("math_div");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context,
                                                                         mergedTestees,
                                                                         filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  MutationPointAddress address = mutationPoint->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Mul, mutatedInstruction.getOpcode());
}

TEST(MutationPoint, SimpleTest_NegateConditionOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();

  Context context;
  context.addModule(std::move(ModuleWithTests));
  context.addModule(std::move(ModuleWithTestees));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("max");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context, mergedTestees, filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  ASSERT_TRUE(isa<CmpInst>(mutationPoint->getOriginalValue()));
  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());
  auto mutatedCmpInstruction = cast<CmpInst>(&mutatedInstruction);
  ASSERT_EQ(mutatedCmpInstruction->getPredicate(), CmpInst::Predicate::ICMP_SGE);
}

TEST(MutationPoint, SimpleTest_AndOrMutator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ANDORReplacement_Module();

  Context context;
  context.addModule(std::move(module));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<AndOrReplacementMutator>());
  MutationsFinder finder(std::move(mutators), config);
  Filter filter;

  Function *testeeFunction = context.lookupDefinedFunction("testee_AND_operator_2branches");
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context,
                                                                         mergedTestees,
                                                                         filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();
  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<BranchInst>(&mutatedInstruction));
}

TEST(MutationPoint, SimpleTest_ScalarValueMutator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ScalarValue_Module();

  Context context;
  context.addModule(std::move(module));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ScalarValueMutator>());
  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("scalar_value");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context, mergedTestees, filter);
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

  auto &mutatedInstruction = mutationPoint1->getAddress().findInstruction(mutationPoint1->getOriginalFunction());
  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

TEST(MutationPoint, SimpleTest_ReplaceCallMutator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ReplaceCall_Module();

  Context context;
  context.addModule(std::move(module));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ReplaceCallMutator>());

  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("replace_call");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context, mergedTestees, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint->getAddress();
  ASSERT_TRUE(isa<CallInst>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
}

TEST(MutationPoint, SimpleTest_ReplaceAssignmentMutator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ReplaceAssignment_Module();

  Context context;
  context.addModule(std::move(module));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ReplaceAssignmentMutator>());

  MutationsFinder finder(std::move(mutators), config);

  Function *testeeFunction = context.lookupDefinedFunction("replace_assignment");
  ASSERT_FALSE(testeeFunction->empty());
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(context, mergedTestees, filter);

  EXPECT_EQ(2U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints[0];
  EXPECT_TRUE(isa<StoreInst>(mutationPoint->getOriginalValue()));

  mutationPoint->setMutatedFunction(mutationPoint->getOriginalFunction());
  mutationPoint->applyMutation();

  auto &mutatedInstruction = mutationPoint->getAddress().findInstruction(mutationPoint->getOriginalFunction());
  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

