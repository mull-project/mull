
#include "Context.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "MutationEngine.h"
#include "MutationOperators/AddMutationOperator.h"
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

Instruction *getFirstNamedInstruction(Function &F, const StringRef &Name) {
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction &Instr = *I;

    if (Instr.getName().equals(Name)) {
      return &*I;
    }
  }

  return nullptr;
}

TEST(MutationEngine, applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  auto Tests = Finder.findTests();

  auto &Test = *(Tests.begin());

  ArrayRef<Function *> Testees = Finder.findTestees(Test.get());

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = *(Testees.begin());
  ASSERT_FALSE(Testee->empty());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  ASSERT_EQ(&MutOp, MP->getOperator());
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

  ASSERT_EQ(MP->getMutatedValue(), NewInstruction);
}

TEST(MutationEngine, applyAndRevertMutation) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  auto Tests = Finder.findTests();

  auto &Test = *(Tests.begin());

  ArrayRef<Function *> Testees = Finder.findTestees(Test.get());

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = *(Testees.begin());
  ASSERT_FALSE(Testee->empty());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  ASSERT_EQ(&MutOp, MP->getOperator());
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  Instruction *ReplacedInstruction = cast<BinaryOperator>(MP->getOriginalValue());
  BasicBlock *ReplacedInstructionParent = ReplacedInstruction->getParent();

  std::string ReplacedInstructionName = ReplacedInstruction->getName().str();

  MutationEngine Engine;

  Engine.applyMutation(Testee->getParent(), *MP);

  // After mutation applied on instruction it should be erased
  Instruction *OldInstruction = cast<BinaryOperator>(MP->getOriginalValue());
  ASSERT_EQ(nullptr, OldInstruction->getParent());

//  Function *MutatedFunction = ModuleWithTestees->getFunction(Testee->getName());

  // After mutation we should have new instruction with the same name as an original instruction
  Instruction *NewInstruction = getFirstNamedInstruction(*Testee, ReplacedInstructionName);
  ASSERT_TRUE(isa<BinaryOperator>(NewInstruction));
  ASSERT_EQ(Instruction::Sub, NewInstruction->getOpcode());

  ASSERT_EQ(MP->getMutatedValue(), NewInstruction);

  Engine.revertMutation(*MP);

  // After mutation is reverted back we should have
  // a new instruction with the same name as an original instruction
  Instruction *RolledBackInstruction = getFirstNamedInstruction(*Testee, ReplacedInstructionName);
  ASSERT_TRUE(isa<BinaryOperator>(RolledBackInstruction));
  ASSERT_EQ(Instruction::Add, RolledBackInstruction->getOpcode());

  // After mutation is reverted back instruction should have the same parent again
  ASSERT_NE(nullptr, RolledBackInstruction->getParent());
  ASSERT_EQ(ReplacedInstructionParent, RolledBackInstruction->getParent());

  ASSERT_EQ(MP->getOriginalValue(), RolledBackInstruction);
}
