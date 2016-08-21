#include "GoogleTest/GoogleTestFinder.h"

#include "Context.h"
#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include "GoogleTest/GoogleTest_Test.h"

#include <vector>

using namespace Mutang;
using namespace llvm;

std::vector<std::unique_ptr<Test>> GoogleTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &M : Ctx.getModules()) {
    auto &x = M->getFunctionList();
    for (auto &Fn : x) {
      if (Fn.getName().endswith("_Test8TestBodyEv")) {
        tests.emplace_back(make_unique<GoogleTest_Test>(&Fn));
      }
    }
  }

  return tests;
}

std::vector<llvm::Function *> GoogleTestFinder::findTestees(Test *Test, Context &Ctx) {
  return std::vector<Function *>();
}

std::vector<std::unique_ptr<MutationPoint>> GoogleTestFinder::findMutationPoints(
                             std::vector<MutationOperator *> &MutationOperators,
                             llvm::Function &F) {
  return std::vector<std::unique_ptr<MutationPoint>>();
}
