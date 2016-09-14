#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "MutationPoint.h"

#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include "SimpleTest/SimpleTest_Test.h"

#include <algorithm>
#include <vector>

using namespace Mutang;
using namespace llvm;

std::vector<std::unique_ptr<Test>> SimpleTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &M : Ctx.getModules()) {
    auto &x = M->getFunctionList();
    for (auto &Fn : x) {
      if (Fn.getName().startswith("test_")) {
        tests.emplace_back(make_unique<SimpleTest_Test>(&Fn));
      }
    }
  }

  return tests;
}

std::vector<Function *> SimpleTestFinder::findTestees(Test *Test, Context &Ctx) {
  SimpleTest_Test *SimpleTest = dyn_cast<SimpleTest_Test>(Test);

  Function &F = *(SimpleTest->GetTestFunction());

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
                          std::vector<MutationOperator *> &MutationOperators,
                          llvm::Function &F) {
  std::vector<std::unique_ptr<MutationPoint>> MutPoints;


  Module *PM = F.getParent();

  auto FII = std::find_if(PM->begin(), PM->end(),
                          [&F] (llvm::Function &f) {
                            return &f == &F;
                          });

  assert(FII != PM->end() && "Expected function to be found in module");
  int FIndex = std::distance(PM->begin(), FII);

  for (Function::iterator B = F.begin(), E = F.end(); B != E; ++B) {
    BasicBlock &BB = *B;

    for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
      Instruction &Instr = *I;

      for (auto &MutOp : MutationOperators) {
        if (MutOp->canBeApplied(Instr)) {
          BasicBlock *BB = Instr.getParent();

          auto BII = std::find_if(F.begin(), F.end(),
                                  [&BB] (llvm::BasicBlock &bb) {
                                    return &bb == BB;
                                  });
          assert(BII != F.end() && "Expected block to be found in function");

          int BBIndex = std::distance(F.begin(), BII);

          int IIndex = std::distance(B->begin(), I);

          MutationPointAddress Address(FIndex, BBIndex, IIndex);

          MutPoints.emplace_back(make_unique<MutationPoint>(MutOp, Address, &Instr));
        }
      }
    }
  }


  return MutPoints;
}
