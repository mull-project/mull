
#include "Context.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "MutationEngine.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "TestModuleFactory.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace Mutang;
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

TEST(MutationEngine, SimpleTest_AddOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mutangModuleWithTests   = make_unique<MutangModule>(std::move(ModuleWithTests), "");
  auto mutangModuleWithTestees = make_unique<MutangModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));
  Ctx.addModule(std::move(mutangModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto Tests = Finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  ArrayRef<Testee> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = Testees.begin()->getFunction();
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  MutationEngine Engine;

  Engine.applyMutation(Testee->getParent(), *MP);

  // After mutation applied on instruction it should be erased
  Instruction *OldInstruction = cast<BinaryOperator>(MP->getOriginalValue());
  ASSERT_EQ(nullptr, OldInstruction->getParent());

//  Function *MutatedFunction = Testee;//ModuleWithTestees->getFunction(Testee->getName());

  // After mutation we should have new instruction with the same name as an original instruction
  Instruction *NewInstruction = getFirstNamedInstruction(*Testee, ReplacedInstructionName);
  ASSERT_TRUE(isa<BinaryOperator>(NewInstruction));
  ASSERT_EQ(Instruction::Sub, NewInstruction->getOpcode());
}

TEST(MutationEngine, SimpleTest_NegateConditionOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();

  auto mutangModuleWithTests   = make_unique<MutangModule>(std::move(ModuleWithTests), "");
  auto mutangModuleWithTestees = make_unique<MutangModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));
  Ctx.addModule(std::move(mutangModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  ArrayRef<Testee> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = Testees.begin()->getFunction();
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress address = MP->getAddress();

  MutationEngine Engine;

  ASSERT_EQ(&FunctionInstructionByAddress(*Testee, address), MP->getOriginalValue());
  Engine.applyMutation(Testee->getParent(), *MP);

  // After mutation applied on instruction it should be erased
  Instruction *OldInstruction = cast<CmpInst>(MP->getOriginalValue());
  ASSERT_EQ(nullptr, OldInstruction->getParent());

  llvm::Instruction &NewInstruction = FunctionInstructionByAddress(*Testee, address);
  ASSERT_TRUE(isa<ICmpInst>(NewInstruction));
  ASSERT_EQ(ICmpInst::ICMP_SGE, (cast<CmpInst>(NewInstruction)).getPredicate());
}
