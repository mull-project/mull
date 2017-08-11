#include "XCTest/XCTestFinder.h"

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

#include "XCTest/XCTest_Test.h"

#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"

#include <queue>
#include <set>
#include <vector>

using namespace mull;
using namespace llvm;

using namespace std;

struct swift_test_demangled {
  string testModule;
  string testSuite;
  string testName;

  string toString() {
    return testModule + "/" + testSuite + "." + testName;
  }
};

swift_test_demangled parseMangledTestName(const string &testName) {
  vector<string> strings;
  string digits;

  for (auto it = testName.begin() + 4; it != testName.end(); it++) {
    auto ch = *it;

    if (isdigit(ch)) {
      digits += ch;
    } else if (digits.size() == 0) {
      continue;
    } else {
      auto length = stoul(digits);
      string chunk(it, it + length);
      strings.push_back(chunk);
      it += length - 1;
      digits = "";
    }
  }

  swift_test_demangled t;
  t.testModule = strings[0];
  t.testSuite = strings[1];
  t.testName = strings[2];

  return t;
}

class XCTestMutationOperatorFilter : public MutationOperatorFilter {
public:
  bool shouldSkipInstruction(llvm::Instruction *instruction) {
    auto debug = instruction->getDebugLoc().get();
    if (!debug) {
      return true;
    }

    if (debug->getLine() == 0) {
      return true;
    }

    return false;
  };
};

XCTestFinder::XCTestFinder(std::vector<std::unique_ptr<MutationOperator>> mutationOperators,
                           const std::vector<std::string> testsToFilter) : TestFinder(),
  mutationOperators(std::move(mutationOperators)),
  testsToFilter(testsToFilter)
  {}

std::vector<std::unique_ptr<Test>> XCTestFinder::findTests(Context &context) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &module : context.getModules()) {
    auto &functionList = module->getModule()->getFunctionList();

    std::vector<Type *> testTypes;
    for (auto &tt : module->getModule()->getIdentifiedStructTypes()) {
      if (tt->getNumElements() >= 6) {
        auto secondType = tt->getElementType(1);
        if (secondType->isStructTy() && secondType->getStructName().startswith("GSqC6XCTest9XCTestRun")) {
          testTypes.push_back(tt);
        }
      }
    }

    for (auto &function : functionList) {
      if (function.isDeclaration()) {
        continue;
      }

      if (!function.getName().equals("_TFC16CryptoSwiftTests8AESTests15testAESEncrypt2fT_T_")) {
        continue;
      }

      auto functionType = function.getFunctionType();
      if (functionType->getNumParams() == 0) {
        continue;
      }
      PointerType *pointer = dyn_cast<PointerType>(functionType->getParamType(0));
      if (!pointer) {
        continue;
      }
      unsigned index = 0;
      auto firstParameter = pointer->getTypeAtIndex(index);
      if (!firstParameter->isStructTy()) {
        continue;
      }

      if (std::find(testTypes.begin(), testTypes.end(), firstParameter) == testTypes.end() ) {
        continue;
      }

      if (!function.getName().endswith("fT_T_")) {
        continue;
      }

      /* functionType->dump(); */
      /* firstParameter->dump(); */
      /* Logger::debug() << function.getName() << "\n"; */
      tests.push_back(make_unique<XCTest_Test>(parseMangledTestName(function.getName().str()).toString(), &function));
    }
  }

  return tests;
}

static bool shouldSkipDefinedFunction(llvm::Function *definedFunction) {
  return false;
}

std::vector<std::unique_ptr<Testee>>
XCTestFinder::findTestees(Test *test,
                            Context &Ctx,
                            int maxDistance) {
  XCTest_Test *concnreteTest = dyn_cast<XCTest_Test>(test);

  Function *testFunction = concnreteTest->getFunction();

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

	callInstruction->dump();

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
//                    Logger::debug() << "SwiftTestFinder> found a function: "
  //                                  << definedFunction->getName() << '\n';
          definedFunction =
            Ctx.lookupDefinedFunction(functionOperand->getName());
        } else {
//	Logger::debug() << "asdasd\n";
  //                  Logger::debug() << "SwiftTestFinder> did not find a function: "
    //                                << definedFunction->getName() << '\n';
//	Logger::debug() << "ASDASD\n";
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
            traversees.push(make_unique<Testee>(definedFunction,
                                                callInstruction,
                                                traverseePointer,
                                                mutationDistance + 1));
          }
        } else {
  //         Logger::debug() << "Could not find functnio definition for: " << functionOperand->getName() << "\n";
        }
      }
    }
  }

  return testees;
}

std::vector<MutationPoint *>
XCTestFinder::findMutationPoints(const Context &context,
                                   llvm::Function &testee) {

  if (MutationPointsRegistry.count(&testee) != 0) {
    return MutationPointsRegistry.at(&testee);
  }

  std::vector<MutationPoint *> points;

  XCTestMutationOperatorFilter filter;

  for (auto &mutationOperator : mutationOperators) {
    for (auto point : mutationOperator->getMutationPoints(context, &testee, filter)) {
      points.push_back(point);
      MutationPoints.emplace_back(std::unique_ptr<MutationPoint>(point));
    }
  }

  MutationPointsRegistry.insert(std::make_pair(&testee, points));
  return points;
}

std::vector<std::unique_ptr<MutationPoint>> XCTestFinder::findMutationPoints(
                            std::vector<MutationOperator *> &MutationOperators,
                            llvm::Function &F) {
  return std::vector<std::unique_ptr<MutationPoint>>();
}
