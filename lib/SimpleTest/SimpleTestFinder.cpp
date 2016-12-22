#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"
#include "Testee.h"

#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include "SimpleTest/SimpleTest_Test.h"

#include <algorithm>
#include <queue>
#include <set>
#include <vector>

using namespace Mutang;
using namespace llvm;

static bool shouldSkipDefinedFunction(llvm::Function *definedFunction) {

  //  if (definedFunction->hasMetadata()) {
  //    int debugInfoKindID = 0;
  //    MDNode *debug = definedFunction->getMetadata(debugInfoKindID);
  //    DISubprogram *subprogram = dyn_cast<DISubprogram>(debug);
  //    if (subprogram) {
  //      if (subprogram->getFilename().contains("include/c++/v1")) {
  //        return true;
  //      }
  //    }
  //  }

  return false;
}

SimpleTestFinder::SimpleTestFinder(
    std::vector<std::unique_ptr<MutationOperator>> mutationOperators)
    : TestFinder(), mutationOperators(std::move(mutationOperators)) {}

std::vector<std::unique_ptr<Test>> SimpleTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &module : Ctx.getModules()) {
    auto &x = module->getModule()->getFunctionList();
    for (auto &Fn : x) {
      if (Fn.getName().startswith("test_")) {

        Logger::info() << "SimpleTestFinder::findTests - found function "
                       << Fn.getName() << '\n';

        tests.emplace_back(make_unique<SimpleTest_Test>(&Fn));
      }
    }
  }

  return tests;
}

std::vector<std::unique_ptr<Testee>>
SimpleTestFinder::findTestees(Test *Test, Context &Ctx, int maxDistance) {
  SimpleTest_Test *SimpleTest = dyn_cast<SimpleTest_Test>(Test);

  Function *testFunction = SimpleTest->GetTestFunction();

  std::vector<std::unique_ptr<Testee>> testees;
  std::queue<std::unique_ptr<Testee>> traversees;
  std::set<Function *> checkedFunctions;

  Module *testBodyModule = testFunction->getParent();

  traversees.push(make_unique<Testee>(testFunction, nullptr, nullptr, 0));

  while (!traversees.empty()) {
    std::unique_ptr<Testee> traversee = std::move(traversees.front());
    Testee *traverseePointer = traversee.get();

    traversees.pop();

    Function *traverseeFunction = traversee->getTesteeFunction();
    const int mutationDistance = traversee->getDistance();

    /// If the function we are processing is in the same translation unit
    /// as the test itself, then we are not looking for mutation points
    /// in this function assuming it to be a helper function.
    /// The only exception is the test function itself that is especially
    /// important for path calculations that are done later in SQLiteReporter.
    if (traverseeFunction->getParent() != testBodyModule ||
        traverseeFunction == testFunction) {
      testees.push_back(std::move(traversee));
    }

    /// The function reached the max allowed distance
    /// Hence we don't go deeper
    if (mutationDistance == maxDistance) {
      continue;
    }

    for (auto &BB : *traverseeFunction) {
      for (auto &I : BB) {
        auto *instruction = &I;

        CallInst *callInstruction = dyn_cast<CallInst>(instruction);
        if (callInstruction == nullptr) {
          continue;
        }

        int callOperandIndex = callInstruction->getNumOperands() - 1;
        Value *callOperand = callInstruction->getOperand(callOperandIndex);
        Function *functionOperand = dyn_cast<Function>(callOperand);

        if (!functionOperand) {
          continue;
        }

        /// Two modules may have static function with the same name, e.g.:
        ///
        ///   // ModuleA
        ///   define range() {
        ///     // ...
        ///   }
        ///
        ///   define test_A() {
        ///     call range()
        ///   }
        ///
        ///   // ModuleB
        ///   define range() {
        ///     // ...
        ///   }
        ///
        ///   define test_B() {
        ///     call range()
        ///   }
        ///
        /// Depending on the order of processing either `range` from `A` or `B`
        /// will be added to the `context`, hence we may find function `range`
        /// from module `B` while processing body of the `test_A`.
        /// To avoid this problem we first look for function inside of a current
        /// module.
        ///
        /// FIXME: Context should report if a function being added already exist
        /// FIXME: What other problems such behaviour may bring?

        Function *definedFunction =
            testBodyModule->getFunction(functionOperand->getName());

        if (!definedFunction || definedFunction->isDeclaration()) {
          definedFunction =
              Ctx.lookupDefinedFunction(functionOperand->getName());
        }

        if (definedFunction) {
          auto functionWasNotProcessed =
              checkedFunctions.find(definedFunction) == checkedFunctions.end();
          checkedFunctions.insert(definedFunction);

          if (functionWasNotProcessed) {
            /// Filtering
            if (shouldSkipDefinedFunction(definedFunction)) {
              continue;
            }

            /// The code below is not actually correct
            /// For each C++ constructor compiler can generate up to three
            /// functions*. Which means that the distance might be incorrect
            /// We need to find a clever way to fix this problem
            ///
            /// * Here is a good overview of what's going on:
            /// http://stackoverflow.com/a/6921467/829116
            ///
            traversees.push(make_unique<Testee>(definedFunction,
                                                callInstruction,
                                                traverseePointer,
                                                mutationDistance + 1));
          }
        }
      }
    }
  }

  return testees;
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

          Logger::info() << "Found Mutation point at address: " << FIndex << ' '
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

std::vector<std::unique_ptr<MutationPoint>>
SimpleTestFinder::findMutationPoints(
    std::vector<MutationOperator *> &MutationOperators, llvm::Function &F) {

  assert(false && "should not reach here");
  std::vector<std::unique_ptr<MutationPoint>> MutPoints;

  /*

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

          MutPoints.emplace_back(make_unique<MutationPoint>(MutOp, Address,
  &Instr));
        }
      }
    }
  }
  */

  return MutPoints;
}
