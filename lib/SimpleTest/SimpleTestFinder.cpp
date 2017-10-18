#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"

#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>

#include "SimpleTest/SimpleTest_Test.h"

#include <algorithm>
#include <queue>
#include <set>
#include <vector>

using namespace mull;
using namespace llvm;

SimpleTestFinder::SimpleTestFinder(
    std::vector<std::unique_ptr<MutationOperator>> mutationOperators)
    : TestFinder(), mutationOperators(std::move(mutationOperators)) {}

std::vector<std::unique_ptr<Test>> SimpleTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &module : Ctx.getModules()) {
    auto &x = module->getModule()->getFunctionList();
    for (auto &Fn : x) {

      /// We find C functions having test_ and the same functions if they are
      /// compiled with C++ (mangled as "_Z25test_").
      if (Fn.getName().find("test_") != std::string::npos) {

        Logger::info() << "SimpleTestFinder::findTests - found function "
                       << Fn.getName() << '\n';

        tests.emplace_back(make_unique<SimpleTest_Test>(&Fn));
      }
    }
  }

  return tests;
}

std::vector<MutationPoint *>
SimpleTestFinder::findMutationPoints(const Context &context,
                                     llvm::Function &F) {
  std::vector<MutationPoint *> MutPoints;

  Module *PM = F.getParent();

  auto FIt = std::find_if(PM->begin(), PM->end(),
                          [&F](llvm::Function &f) { return &f == &F; });

  assert(FIt != PM->end() && "Expected function to be found in module");
  int FIndex = std::distance(PM->begin(), FIt);

  for (auto &BB : F) {
    for (auto &I : BB) {

      for (auto &MutOp : mutationOperators) {
        if (MutOp->canBeApplied(I)) {

          auto BBIt =
              std::find_if(F.begin(), F.end(),
                           [&BB](llvm::BasicBlock &bb) { return &bb == &BB; });
          auto IIt =
              std::find_if(BB.begin(), BB.end(),
                           [&I](llvm::Instruction &i) { return &i == &I; });

          assert(BBIt != F.end() && "Expected block to be found in function");

          int BBIndex = std::distance(F.begin(), BBIt);
          int IIndex = std::distance(BB.begin(), IIt);

          MutationPointAddress Address(FIndex, BBIndex, IIndex);

          Logger::info() << "SimpleTestFinder> Found Mutation point at address: "
                         << FIndex << ' '
                         << BBIndex << ' ' << IIndex << '\n';

          auto module = context.moduleWithIdentifier(
              I.getModule()->getModuleIdentifier());

          MutationPoints.emplace_back(
              make_unique<MutationPoint>(MutOp.get(), Address, &I, module));
          MutPoints.push_back(MutationPoints.back().get());
        }
      }
    }
  }

  MutationPointsRegistry.insert(std::make_pair(&F, MutPoints));

  return MutPoints;
}
