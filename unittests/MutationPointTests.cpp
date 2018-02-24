#include "Context.h"
#include "MutationOperators/MathAddMutator.h"
#include "MutationOperators/MathDivMutator.h"
#include "MutationOperators/MathMulMutator.h"
#include "MutationOperators/MathSubMutator.h"
#include "MutationOperators/NegateConditionMutator.h"
#include "MutationOperators/AndOrReplacementMutator.h"
#include "MutationOperators/ScalarValueMutator.h"
#include "MutationOperators/ReplaceAssignmentMutator.h"
#include "MutationOperators/ReplaceCallMutator.h"
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

static llvm::Instruction &FunctionInstructionByAddress(Function &F, MutationPointAddress address) {
  printf("ModuleInstructionByAddress: %d %d %d\n",
         address.getFnIndex(),
         address.getBBIndex(),
         address.getIIndex());

  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &NewInstruction = *(std::next(B.begin(), address.getIIndex()));

  return NewInstruction;
}

TEST(MutationPoint, SimpleTest_AddOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_CountLetters_Module();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 0);

  Filter filter;
  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx,
                                                                         testee,
                                                                         filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  LLVMContext localContext;
  auto ownedMutatedModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("count_letters");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->getModule()->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));
  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Sub, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathSubOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathSub_Module();

  Context Ctx;
  Ctx.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathSubMutator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("math_sub");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  LLVMContext localContext;
  auto ownedMutatedModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("math_sub");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->getModule()->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Add, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathMulOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathMul_Module();

  Context Ctx;
  Ctx.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathMulMutator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("math_mul");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);

  ASSERT_EQ(mutationPoints.size(), 1UL);

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  LLVMContext localContext;
  auto ownedMutatedModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("math_mul");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->getModule()->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::SDiv, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathDivOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathDiv_Module();

  Context Ctx;
  Ctx.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathDivMutator>());

  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("math_div");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx,
                                                                         testee,
                                                                         filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  LLVMContext localContext;
  auto ownedMutatedModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("math_div");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->getModule()->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Mul, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_NegateConditionOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("max");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress address = MP->getAddress();

  ASSERT_EQ(&FunctionInstructionByAddress(*testeeFunction, address), MP->getOriginalValue());

  LLVMContext localContext;
  auto ownedMutatedModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("max");
  ASSERT_TRUE(mutatedTestee != nullptr);

  auto &mutatedInstruction = FunctionInstructionByAddress(*mutatedTestee,
                                                          MP->getAddress());
  ASSERT_TRUE(CmpInst::classof(&mutatedInstruction));

  auto mutatedCmpInstruction = cast<CmpInst>(&mutatedInstruction);
  ASSERT_EQ(mutatedCmpInstruction->getPredicate(), CmpInst::Predicate::ICMP_SGE);
}

TEST(MutationPoint, SimpleTest_AndOrMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ANDORReplacement_Module();

  Context ctx;
  ctx.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutator>());

  MutationsFinder finder(std::move(mutationOperators));
  Filter filter;

  {
    Function *testeeFunction = ctx.lookupDefinedFunction("testee_AND_operator_2branches");
    Testee testee(testeeFunction, nullptr, 1);

    std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(ctx,
                                                                           testee,
                                                                           filter);

    ASSERT_EQ(1U, mutationPoints.size());

    MutationPoint *mutationPoint = (*(mutationPoints.begin()));

    MutationPointAddress address = mutationPoint->getAddress();

    ASSERT_TRUE(isa<BranchInst>(mutationPoint->getOriginalValue()));

    ASSERT_EQ(&FunctionInstructionByAddress(*testeeFunction,
                                            address),
                                            mutationPoint->getOriginalValue());

    LLVMContext localContext;
    auto ownedMutatedModule = mutationPoint->getOriginalModule()->clone(localContext);
    mutationPoint->applyMutation(*ownedMutatedModule.get());

    Function *mutatedTesteeFunction = ownedMutatedModule->getModule()->getFunction("testee_AND_operator_2branches");
    ASSERT_TRUE(mutatedTesteeFunction != nullptr);

    auto &mutatedInstruction = FunctionInstructionByAddress(*mutatedTesteeFunction, address);

    ASSERT_TRUE(BranchInst::classof(&mutatedInstruction));

    auto mutatedBranchInstruction = cast<BranchInst>(&mutatedInstruction);
    ASSERT_TRUE(mutatedBranchInstruction != nullptr);
  }
}

TEST(MutationPoint, SimpleTest_ScalarValueMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ScalarValue_Module();

  Context Ctx;
  Ctx.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<ScalarValueMutator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("scalar_value");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);
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

  LLVMContext localContext;
  auto ownedMutatedModule = mutationPoint1->getOriginalModule()->clone(localContext);
  mutationPoint1->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("scalar_value");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->getModule()->begin(), mutationPointAddress1.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), mutationPointAddress1.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), mutationPointAddress1.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

TEST(MutationPoint, SimpleTest_ReplaceCallMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ReplaceCall_Module();

  Context Ctx;
  Ctx.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<ReplaceCallMutator>());

  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("replace_call");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, nullptr, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint1 = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint1->getAddress();
  ASSERT_TRUE(isa<CallInst>(mutationPoint1->getOriginalValue()));

  LLVMContext localContext;
  auto ownedMutatedModule = mutationPoint1->getOriginalModule()->clone(localContext);
  mutationPoint1->applyMutation(*ownedMutatedModule.get());

  Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("replace_call");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Instruction &instructionByMutationAddress =
    mutationPointAddress1.findInstruction(ownedMutatedModule->getModule());

  ASSERT_TRUE(isa<BinaryOperator>(instructionByMutationAddress));
}

TEST(MutationPoint, SimpleTest_ReplaceAssignmentMutationOperator_applyMutation) {
    auto module = TestModuleFactory.create_SimpleTest_ReplaceAssignment_Module();

    Context Ctx;
    Ctx.addModule(std::move(module));

    std::vector<std::unique_ptr<Mutator>> mutationOperators;
    mutationOperators.emplace_back(make_unique<ReplaceAssignmentMutator>());

    MutationsFinder finder(std::move(mutationOperators));

    Function *testeeFunction = Ctx.lookupDefinedFunction("replace_assignment");
    ASSERT_FALSE(testeeFunction->empty());
    Testee testee(testeeFunction, nullptr, 1);
    Filter filter;

    std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);

    EXPECT_EQ(2U, mutationPoints.size());

    MutationPoint *mutationPoint1 = mutationPoints[0];
    MutationPointAddress mutationPointAddress1 = mutationPoint1->getAddress();
    EXPECT_TRUE(isa<StoreInst>(mutationPoint1->getOriginalValue()));

    LLVMContext localContext;
    auto ownedMutatedModule = mutationPoint1->getOriginalModule()->clone(localContext);
    mutationPoint1->applyMutation(*ownedMutatedModule.get());

    Function *mutatedTestee = ownedMutatedModule->getModule()->getFunction("replace_assignment");
    ASSERT_TRUE(mutatedTestee != nullptr);

    llvm::Instruction &instructionByMutationAddress =
    mutationPointAddress1.findInstruction(ownedMutatedModule->getModule());

    ASSERT_TRUE(isa<StoreInst>(instructionByMutationAddress));
}
