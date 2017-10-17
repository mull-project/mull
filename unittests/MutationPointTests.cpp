#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/MathDivMutationOperator.h"
#include "MutationOperators/MathMulMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/ScalarValueMutationOperator.h"
#include "MutationOperators/ReplaceCallMutationOperator.h"
#include "TestModuleFactory.h"
#include "Testee.h"
#include "MutationsFinder.h"
#include "Filter.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

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

Instruction *getFirstNamedInstruction(Function &F, const StringRef &Name) {
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction &Instr = *I;

    //printf("Found instruction: %s\n", Instr.getName().str().c_str());

    if (Instr.getName().equals(Name)) {
      return &*I;
    }
  }

  return nullptr;
}

TEST(MutationPoint, SimpleTest_AddOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 0);

  Filter filter;
  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx,
                                                                         testee,
                                                                         filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("count_letters");
  ASSERT_TRUE(mutatedTestee != nullptr);

  Instruction *mutatedInstruction = getFirstNamedInstruction(*mutatedTestee, "add");
  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Sub, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathSubOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathSub_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathSubMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("math_sub");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("math_sub");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Add, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathMulOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathMul_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathMulMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("math_mul");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);

  ASSERT_EQ(mutationPoints.size(), 1UL);

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("math_mul");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::SDiv, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathDivOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathDiv_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathDivMutationOperator>());

  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("math_div");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx,
                                                                         testee,
                                                                         filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("math_div");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->begin(), address.getFnIndex()));
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

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("max");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);
  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *MP = (*(mutationPoints.begin()));
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress address = MP->getAddress();

  ASSERT_EQ(&FunctionInstructionByAddress(*testeeFunction, address), MP->getOriginalValue());

  auto ownedMutatedTesteeModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedTesteeModule->getFunction("max");
  ASSERT_TRUE(mutatedTestee != nullptr);

  auto &mutatedInstruction = FunctionInstructionByAddress(*mutatedTestee,
                                                          MP->getAddress());
  ASSERT_TRUE(CmpInst::classof(&mutatedInstruction));

  auto mutatedCmpInstruction = cast<CmpInst>(&mutatedInstruction);
  ASSERT_EQ(mutatedCmpInstruction->getPredicate(), CmpInst::Predicate::ICMP_SGE);
}

TEST(MutationPoint, SimpleTest_AndOrMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ANDORReplacement_Module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context ctx;
  ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());

  MutationsFinder finder(std::move(mutationOperators));
  Filter filter;

  {
    Function *testeeFunction = ctx.lookupDefinedFunction("testee_AND_operator_2branches");
    Testee testee(testeeFunction, 1);

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

    auto ownedMutatedTesteeModule = mutationPoint->cloneModuleAndApplyMutation();

    Function *mutatedTesteeFunction = ownedMutatedTesteeModule->getFunction("testee_AND_operator_2branches");
    ASSERT_TRUE(mutatedTesteeFunction != nullptr);

    auto &mutatedInstruction = FunctionInstructionByAddress(*mutatedTesteeFunction, address);

    ASSERT_TRUE(BranchInst::classof(&mutatedInstruction));

    auto mutatedBranchInstruction = cast<BranchInst>(&mutatedInstruction);
    ASSERT_TRUE(mutatedBranchInstruction != nullptr);
  }
}

TEST(MutationPoint, SimpleTest_ScalarValueMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ScalarValue_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<ScalarValueMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("scalar_value");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 1);
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

  auto ownedMutatedModule = mutationPoint1->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("scalar_value");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->begin(), mutationPointAddress1.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), mutationPointAddress1.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), mutationPointAddress1.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}

TEST(MutationPoint, SimpleTest_ReplaceCallMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ReplaceCall_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<ReplaceCallMutationOperator>());

  MutationsFinder finder(std::move(mutationOperators));

  Function *testeeFunction = Ctx.lookupDefinedFunction("replace_call");
  ASSERT_FALSE(testeeFunction->empty());
  Testee testee(testeeFunction, 1);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints = finder.getMutationPoints(Ctx, testee, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint1 = mutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint1->getAddress();
  ASSERT_TRUE(isa<CallInst>(mutationPoint1->getOriginalValue()));

  auto ownedMutatedModule = mutationPoint1->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("replace_call");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Instruction &instructionByMutationAddress =
    mutationPointAddress1.findInstruction(ownedMutatedModule.get());

  ASSERT_TRUE(isa<BinaryOperator>(instructionByMutationAddress));
}
