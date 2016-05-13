#include "TestFinders/SimpleTestFinder.h"
#include "MutationEngine.h"
#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static LLVMContext Ctx;

static std::unique_ptr<Module> parseIR(const char *IR) {
  SMDiagnostic Err;
  return parseAssemblyString(IR, Err, Ctx);
}

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
  auto ModuleWithTests = parseIR("declare i32 @sum(i32, i32)"
                                 "define i32 @test_main() {\n"
                                 "entry:\n"
                                 "  %result = alloca i32, align 4\n"
                                 "  %result_matches = alloca i32, align 4\n"
                                 "  %call = call i32 @sum(i32 3, i32 5)\n"
                                 "  store i32 %call, i32* %result, align 4\n"
                                 "  %0 = load i32, i32* %result, align 4\n"
                                 "  %cmp = icmp eq i32 %0, 8\n"
                                 "  %conv = zext i1 %cmp to i32\n"
                                 "  store i32 %conv, i32* %result_matches, align 4\n"
                                 "  %1 = load i32, i32* %result_matches, align 4\n"
                                 "  ret i32 %1\n"
                                 "}\n");

  auto ModuleWithTestees = parseIR("define i32 @sum(i32 %a, i32 %b) {\n"
                                   "entry:\n"
                                   "  %a.addr = alloca i32, align 4\n"
                                   "  %b.addr = alloca i32, align 4\n"
                                   "  store i32 %a, i32* %a.addr, align 4\n"
                                   "  store i32 %b, i32* %b.addr, align 4\n"
                                   "  %0 = load i32, i32* %a.addr, align 4\n"
                                   "  %1 = load i32, i32* %b.addr, align 4\n"
                                   "  %add = add nsw i32 %0, %1\n"
                                   "  ret i32 %add\n"
                                   "}");

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  Function *Test = *(Tests.begin());

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);

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

  Engine.applyMutation(*MP);

  // After mutation applied on instruction it should be erased
  Instruction *OldInstruction = cast<BinaryOperator>(MP->getOriginalValue());
  ASSERT_EQ(nullptr, OldInstruction->getParent());

  Function *MutatedFunction = ModuleWithTestees->getFunction(Testee->getName());

  // After mutation we should have new instruction with the same name as an original instruction
  Instruction *NewInstruction = getFirstNamedInstruction(*MutatedFunction, ReplacedInstructionName);
  ASSERT_TRUE(isa<BinaryOperator>(NewInstruction));
  ASSERT_EQ(Instruction::Sub, NewInstruction->getOpcode());

  ASSERT_EQ(MP->getMutatedValue(), NewInstruction);
}

TEST(MutationEngine, applyAndRevertMutation) {
  auto ModuleWithTests = parseIR("declare i32 @sum(i32, i32)"
                                 "define i32 @test_main() {\n"
                                 "entry:\n"
                                 "  %result = alloca i32, align 4\n"
                                 "  %result_matches = alloca i32, align 4\n"
                                 "  %call = call i32 @sum(i32 3, i32 5)\n"
                                 "  store i32 %call, i32* %result, align 4\n"
                                 "  %0 = load i32, i32* %result, align 4\n"
                                 "  %cmp = icmp eq i32 %0, 8\n"
                                 "  %conv = zext i1 %cmp to i32\n"
                                 "  store i32 %conv, i32* %result_matches, align 4\n"
                                 "  %1 = load i32, i32* %result_matches, align 4\n"
                                 "  ret i32 %1\n"
                                 "}\n");

  auto ModuleWithTestees = parseIR("define i32 @sum(i32 %a, i32 %b) {\n"
                                   "entry:\n"
                                   "  %a.addr = alloca i32, align 4\n"
                                   "  %b.addr = alloca i32, align 4\n"
                                   "  store i32 %a, i32* %a.addr, align 4\n"
                                   "  store i32 %b, i32* %b.addr, align 4\n"
                                   "  %0 = load i32, i32* %a.addr, align 4\n"
                                   "  %1 = load i32, i32* %b.addr, align 4\n"
                                   "  %add = add nsw i32 %0, %1\n"
                                   "  ret i32 %add\n"
                                   "}");

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  Function *Test = *(Tests.begin());

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);

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

  Engine.applyMutation(*MP);

  // After mutation applied on instruction it should be erased
  Instruction *OldInstruction = cast<BinaryOperator>(MP->getOriginalValue());
  ASSERT_EQ(nullptr, OldInstruction->getParent());

  Function *MutatedFunction = ModuleWithTestees->getFunction(Testee->getName());

  // After mutation we should have new instruction with the same name as an original instruction
  Instruction *NewInstruction = getFirstNamedInstruction(*MutatedFunction, ReplacedInstructionName);
  ASSERT_TRUE(isa<BinaryOperator>(NewInstruction));
  ASSERT_EQ(Instruction::Sub, NewInstruction->getOpcode());

  ASSERT_EQ(MP->getMutatedValue(), NewInstruction);

  Engine.revertMutation(*MP);

  // After mutation rolled back instruction should have parent again
  ASSERT_NE(nullptr, OldInstruction->getParent());

  // After mutation we should have new instruction with the same name as an original instruction
  Instruction *RolledBackInstruction = getFirstNamedInstruction(*MutatedFunction, ReplacedInstructionName);
  ASSERT_TRUE(isa<BinaryOperator>(RolledBackInstruction));
  ASSERT_EQ(Instruction::Add, RolledBackInstruction->getOpcode());

  ASSERT_EQ(MP->getOriginalValue(), RolledBackInstruction);
}
