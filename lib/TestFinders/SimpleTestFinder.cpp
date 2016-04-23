#include "TestFinders/SimpleTestFinder.h"

#include "Context.h"
#include "MutationOperators/MutationOperator.h"


#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include <vector>

using namespace Mutang;
using namespace llvm;

ArrayRef<Function *> SimpleTestFinder::findTests() {
  std::vector<Function *> tests;

  for (auto &M : Ctx.getModules()) {
    for (auto &Fn : M->getFunctionList()) {
      if (Fn.getName().startswith("test_")) {
        tests.push_back(&Fn);
      }
    }
  }

  return tests;
}

ArrayRef<Function *> SimpleTestFinder::findTestees(llvm::Function &F) {
  std::vector<Function *> testees;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
      Value *V = CI->getOperand(CI->getNumOperands() - 1);
      Function *F = cast_or_null<Function>(V);

      /// FIXME: Export all declared functions to external registry
      /// while looking for tests
      for (auto &M : Ctx.getModules()) {
        for (auto &Fn : M->getFunctionList()) {
          if (Fn.getName() == F->getName()) {
            if (!Fn.empty()) {
              testees.push_back(&Fn);
            }
          }
        }
      }

    }
  }

  return testees;
}

std::vector<std::unique_ptr<MutationPoint>> SimpleTestFinder::findMutationPoints(
                          llvm::ArrayRef<MutationOperator *> &MutationOperators,
                          llvm::Function &F) {
  std::vector<std::unique_ptr<MutationPoint>> MutPoints;

  /// FIXME: Iterate over BasicBlocks as well
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction &Instr = *I;

    for (auto &MutOp : MutationOperators) {
      if (MutOp->canBeApplied(Instr)) {
        MutPoints.emplace_back(make_unique<MutationPoint>(MutOp, &Instr));
      }
    }
  }

  return MutPoints;
}
