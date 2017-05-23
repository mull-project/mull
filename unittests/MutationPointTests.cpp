
#include "Context.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"

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

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto Tests = Finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

  Function *Testee = Testees[1]->getTesteeFunction();
  ASSERT_FALSE(Testee->empty());

  ASSERT_EQ(1, Testees[1]->getDistance());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  std::unique_ptr<TargetMachine> targetMachine(
    EngineBuilder().selectTarget(Triple(), "", "",
    SmallVector<std::string, 1>()));
  
  Compiler compiler(*targetMachine.get());

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("count_letters");
  ASSERT_TRUE(mutatedTestee != nullptr);

  Instruction *mutatedInstruction = getFirstNamedInstruction(*mutatedTestee, "add");
  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Sub, mutatedInstruction->getOpcode());
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

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

  Function *Testee = Testees[1]->getTesteeFunction();
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress address = MP->getAddress();

  ASSERT_EQ(&FunctionInstructionByAddress(*Testee, address), MP->getOriginalValue());

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
  auto module = TestModuleFactory.create_SimpleTest_AndOrReplacement_Module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context ctx;
  ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto tests = Finder.findTests(ctx);
  ASSERT_EQ(6U, tests.size());

  {
    auto &test1 = tests[0];

    std::vector<std::unique_ptr<Testee>> test1_testees = Finder.findTestees(test1.get(), ctx, 4);

    ASSERT_EQ(2U, test1_testees.size());

    // testee_AND_operator_2branches()
    // testee #0 is always test itself.
    Function *test1_testee1_function = test1_testees[1]->getTesteeFunction();

    std::vector<MutationPoint *> test1_testee1_mutationPoints =
      Finder.findMutationPoints(ctx, *test1_testee1_function);

    ASSERT_EQ(1U, test1_testee1_mutationPoints.size());

    MutationPoint *test1_testee1_mutationPoint1 = (*(test1_testee1_mutationPoints.begin()));

    MutationPointAddress test1_testee1_mutationPoint1_address =
      test1_testee1_mutationPoint1->getAddress();

    ASSERT_TRUE(isa<BranchInst>(test1_testee1_mutationPoint1->getOriginalValue()));

    ASSERT_EQ(&FunctionInstructionByAddress(*test1_testee1_function,
                                            test1_testee1_mutationPoint1_address),
                                            test1_testee1_mutationPoint1->getOriginalValue());

    auto ownedMutatedTesteeModule = test1_testee1_mutationPoint1->cloneModuleAndApplyMutation();

    Function *test1_mutatedTestee1_function = ownedMutatedTesteeModule->getFunction("testee_AND_operator_2branches");
    ASSERT_TRUE(test1_mutatedTestee1_function != nullptr);

    auto &test1_mutatedTestee1_mutatedInstruction =
      FunctionInstructionByAddress(*test1_mutatedTestee1_function,
                                   test1_testee1_mutationPoint1_address);

    ASSERT_TRUE(BranchInst::classof(&test1_mutatedTestee1_mutatedInstruction));

    auto test1_mutatedTestee1_mutatedBranchInstruction = cast<BranchInst>(&test1_mutatedTestee1_mutatedInstruction);
    ASSERT_TRUE(test1_mutatedTestee1_mutatedBranchInstruction != nullptr);
  }
}
