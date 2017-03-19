#include "Rust/RustTestFinder.h"

#include "Context.h"
#include "Logger.h"
#include "MutationOperators/MutationOperator.h"
#include "MutationOperators/MutationOperatorFilter.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "Rust/RustTest.h"

#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"

#include <queue>
#include <set>
#include <vector>

#include <cxxabi.h>

using namespace mull;
using namespace llvm;

class RustTestMutationOperatorFilter : public MutationOperatorFilter {
public:
  bool shouldSkipInstruction(llvm::Instruction *instruction) {
    if (CallInst *callInst = dyn_cast<CallInst>(instruction)) {
      if (Function *calledFunction = callInst->getCalledFunction()) {
        const std::string &cfName = calledFunction->getName();

        if (cfName.find("panic") != std::string::npos) {
          return true;
        }

        if (cfName.find("llvm.memcpy") != std::string::npos) {
          return true;
        }
      }
    }

    // For now we ignore all instructions without a debug information.
    if (instruction->hasMetadata() == false) {
      return true;
    }

    int debugInfoKindID = 0;
    MDNode *debug = instruction->getMetadata(debugInfoKindID);

    if (DILocation *location = dyn_cast<DILocation>(debug)) {
      if (location->getFilename().str().find("buildbot") != std::string::npos) {
        return true;
      }
    }

    return false;
  };
};

RustTestFinder::RustTestFinder(
  std::vector<std::unique_ptr<MutationOperator>> mutationOperators,
  const std::vector<std::string> testsToFilter) : TestFinder(),
  mutationOperators(std::move(mutationOperators)),
  testsToFilter(testsToFilter)
{
}

/// The algorithm is the following:
///
/// When a Rust module with functions and tests is compiled to LLVM IR via
/// `rustc --test`, the following functions and constants are important:
///
/// - `main` function which is a primary test runner
/// - `test_main` (called by `main`)
/// - `test_main_static` (called by `test_main`, it is the actual test runner),
///   See https://doc.rust-lang.org/1.1.0/src/test/lib.rs.html#267
///
/// Rust stores the test methods to run: their names and function pointers into
/// a global variable: literal of type `internal unnamed_addr constant` which is
/// then given by reference to `test_main_static`.
///
/// Algorithm works in two steps:
/// 1) First we find a `test_main_static` function and find that global
/// variable.
/// 2) We go through that literal and extract the test function pointers.
///
/// See prettified LLVM IR example for simple Rust module with tests:
/// https://github.com/mull-project/mull/blob/54d0e6f4ddea458a5d77fa707b26acef9f5a6093/lab/rust/example.ll.pretty
///
/// See also original StackOverflow question we asked in order to get a feedback
/// about possible approaches:
/// http://stackoverflow.com/questions/42177712/how-do-i-find-the-function-pointers-for-tests-from-the-llvm-ir-code-of-a-rust-pr#42177712
///
/// TODO:
/// `main` function calls `test_main_static` indirectly: the pointer to it
/// is given to a `std::rt::lang_start` function. There might be some important
/// setup going on (setup of global variables, static constructors, etc).
/// The work on real-world Rust code bases will most likely reveal more about
/// this.
std::vector<std::unique_ptr<Test>> RustTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  /// If we have multiple IR modules compiled, each module will have its own
  /// global variable with test functions.
  std::vector<GlobalValue *> testContentsGlobalVariables;

  for (auto &module : Ctx.getModules()) {
    auto &functionList = module->getModule()->getFunctionList();
    for (auto &function : functionList) {
      for (auto &bb : function) {
        for (auto &instruction : bb) {
          CallInst *callInst = dyn_cast<CallInst>(&instruction);

          if (callInst == nullptr) {
            continue;
          }

          Function *calledFunction = callInst->getCalledFunction();

          if (calledFunction == nullptr) {
            continue;
          }

          StringRef cfName = calledFunction->getName();

          if (cfName.find("test_main_static") == std::string::npos) {
            continue;
          }

          assert(callInst->getNumArgOperands() == 2);

          auto operand1 = callInst->getOperand(0);
          assert(operand1->getType()->getTypeID() == llvm::Type::PointerTyID);

          auto ref = dyn_cast<ConstantExpr>(operand1);
          assert(ref);

          auto global = dyn_cast<GlobalValue>(ref->getOperand(0));
          assert(global);

          testContentsGlobalVariables.push_back(global);
        }
      }
    }
  }

  if (testContentsGlobalVariables.size() == 0) {
    return tests;
  }

  for (auto &testContentsGV : testContentsGlobalVariables) {
    assert(testContentsGV->getValueType()->getTypeID() == Type::StructTyID);

    auto testContents = dyn_cast<Constant>(testContentsGV->getOperand(0));
    assert(testContents->getType()->getTypeID() == Type::StructTyID);

    int numberOfTestDescriptions = testContents->getNumOperands();

    for (int i = 0; i < numberOfTestDescriptions; i++) {
      auto testStruct = dyn_cast<Constant>(testContents->getOperand(i));
      assert(testStruct->getType()->getTypeID() == Type::StructTyID);
      assert(testStruct->getNumOperands() == 2);

      auto testPointerStruct =
        dyn_cast<ConstantStruct>(testStruct->getOperand(1));
      assert(testPointerStruct->getNumOperands() == 3);

      auto testFunction = dyn_cast<Function>(testPointerStruct->getOperand(1));

      if (testsToFilter.empty() == false &&
          std::find(testsToFilter.begin(),
                    testsToFilter.end(),
                    testFunction->getName().str()) == testsToFilter.end()) {
        continue;
      }

      tests.push_back(make_unique<RustTest>(testFunction->getName(),
                                            testFunction));
    }
  }

  return tests;
}

static bool shouldSkipDefinedFunction(llvm::Function *definedFunction) {
  // _ZN3std9panicking11begin_panic17h6c6bf73f39cdbdacE
  if (definedFunction->getName().find(StringRef("begin_panic")) != StringRef::npos) {
    return true;
  }

  return false;
}

std::vector<std::unique_ptr<Testee>>
RustTestFinder::findTestees(Test *Test,
                              Context &Ctx,
                              int maxDistance) {
  RustTest *rustTest = dyn_cast<RustTest>(Test);

  Function *testFunction = rustTest->getFunction();

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

    testees.push_back(std::move(traversee));

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
        } else {
//          Logger::debug() << "RustTestFinder> did not find a function: "
//                          << definedFunction->getName() << '\n';
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
RustTestFinder::findMutationPoints(const Context &context,
                                     llvm::Function &testee) {

  if (MutationPointsRegistry.count(&testee) != 0) {
    return MutationPointsRegistry.at(&testee);
  }

  std::vector<MutationPoint *> points;

  RustTestMutationOperatorFilter filter;

  for (auto &mutationOperator : mutationOperators) {
    for (auto point : mutationOperator->getMutationPoints(context, &testee, filter)) {
      points.push_back(point);
      MutationPoints.emplace_back(std::unique_ptr<MutationPoint>(point));
    }
  }

  MutationPointsRegistry.insert(std::make_pair(&testee, points));
  return points;
}

std::vector<std::unique_ptr<MutationPoint>> RustTestFinder::findMutationPoints(
                             std::vector<MutationOperator *> &MutationOperators,
                             llvm::Function &F) {
  return std::vector<std::unique_ptr<MutationPoint>>();
}
