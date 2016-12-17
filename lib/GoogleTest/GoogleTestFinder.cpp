#include "GoogleTest/GoogleTestFinder.h"

#include "Context.h"
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

#include "GoogleTest/GoogleTest_Test.h"

#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"

#include <queue>
#include <set>
#include <vector>

#include <cxxabi.h>

using namespace Mutang;
using namespace llvm;

class GoogleTestMutationOperatorFilter : public MutationOperatorFilter {
public:
  bool shouldSkipInstruction(llvm::Instruction *instruction) {
    if (instruction->hasMetadata()) {
      int debugInfoKindID = 0;
      MDNode *debug = instruction->getMetadata(debugInfoKindID);

      DILocation *location = dyn_cast<DILocation>(debug);
      if (location) {
        if (location->getFilename().contains("include/c++/v1")) {
          return true;
        }
      }
    }

    return false;
  };
};

GoogleTestFinder::GoogleTestFinder() : TestFinder() {
  /// FIXME: should come from outside
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());
  mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());
}

/// The algorithm is the following:
///
/// Each test has an instance of type TestInfo associated with it
/// This instance is stored in a static variable
/// Before program starts this variable gets instantiated by registering
/// in the system.
///
/// The registration made by the following function:
///
///    TestInfo* internal::MakeAndRegisterTestInfo(
///              const char* test_case_name, const char* name,
///              const char* type_param,
///              const char* value_param,
///              internal::TypeId fixture_class_id,
///              Test::SetUpTestCaseFunc set_up_tc,
///              Test::TearDownTestCaseFunc tear_down_tc,
///              internal::TestFactoryBase* factory);
///
/// Here we particularly interested in the values of `test_case_name` and `name`
/// Note:
///   Values `type_param` and `value_param` are also important,
///   but ignored for the moment because we do not support Typed and
///   Value Parametrized tests yet.
///
/// To extract this information we need to find a global
/// variable of type `class.testing::TestInfo`, then find its usage (it used
/// only once), from this usage extract a call to
/// function `MakeAndRegisterTestInfo`, from the call extract values of
/// first and second parameters.
/// Concatenation of thsi parameters is exactly the test name.
/// Note: except of Typed and Value Prametrized Tests
///

std::vector<std::unique_ptr<Test>> GoogleTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &M : Ctx.getModules()) {
    for (auto &Global : M->getModule()->getGlobalList()) {
      Type *Ty = Global.getValueType();
      if (Ty->getTypeID() != Type::PointerTyID) {
        continue;
      }

      Type *SeqTy = Ty->getSequentialElementType();
      if (!SeqTy) {
        continue;
      }

      StructType *STy = dyn_cast<StructType>(SeqTy);
      if (!STy) {
        continue;
      }

      /// If two modules contain the same type, then when second modules is loaded
      /// the typename is changed a bit, e.g.:
      ///
      ///   class.testing::TestInfo     // type from first module
      ///   class.testing::TestInfo.25  // type from second module
      ///
      /// Hence we cannot just compare string, and rather should
      /// compare the beginning of the typename

      if (!STy->getName().startswith(StringRef("class.testing::TestInfo"))) {
        continue;
      }

      /// At this point the Global has only one usage
      /// The user of the Global is part of initialization function
      /// It looks like this:
      ///
      ///   store %"class.testing::TestInfo"* %call2, %"class.testing::TestInfo"** @_ZN16Hello_world_Test10test_info_E
      ///
      /// From here we need to extract actual user, which is a `store` instruction
      /// The `store` instruction uses variable `%call2`, which is created
      /// from the following code:
      ///
      ///   %call2 = call %"class.testing::TestInfo"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.1, i32 0, i32 0), i8* null, i8* null, i8* %call, void ()* @_ZN7testing4Test13SetUpTestCaseEv, void ()* @_ZN7testing4Test16TearDownTestCaseEv, %"class.testing::internal::TestFactoryBase"* %1)
      ///
      /// Which can be roughly simplified to the following pseudo-code:
      ///
      ///   testInfo = MakeAndRegisterTestInfo("Test Suite Name",
      ///                                      "Test Case Name",
      ///                                      setUpFunctionPtr,
      ///                                      tearDownFunctionPtr,
      ///                                      some_other_ignored_parameters)
      ///
      /// Where `testInfo` is exactly the `%call2` from above.
      /// From the `MakeAndRegisterTestInfo` we need to extract test suite
      /// and test case names. Having those in place it's possible to provide
      /// correct filter for GoogleTest framework
      ///
      /// Putting lots of assertions to check the hardway whether
      /// my assumptions are correct or not

      assert(Global.getNumUses() == 1 &&
             "The Global (TestInfo) used only once during test setup");

      auto StoreInstUser = *Global.users().begin();
      assert(isa<StoreInst>(StoreInstUser) &&
             "The Global should only be used within store instruction");

      auto Store = dyn_cast<StoreInst>(StoreInstUser);
      auto ValueOp = Store->getValueOperand();
      assert(isa<CallInst>(ValueOp) &&
             "Store should be using call to MakeAndRegisterTestInfo");

      auto CallInstruction = dyn_cast<CallInst>(ValueOp);

      /// Once we have the CallInstruction we can extract Test Suite Name and Test Case Name
      /// To extract them we need climb to the top, i.e.:
      ///
      ///   i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0)
      ///   i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str1, i32 0, i32 0)

      auto TestSuiteNameConstRef = dyn_cast<ConstantExpr>(CallInstruction->getArgOperand(0));
      assert(TestSuiteNameConstRef);

      auto TestCaseNameConstRef = dyn_cast<ConstantExpr>(CallInstruction->getArgOperand(1));
      assert(TestCaseNameConstRef);

      ///   @.str = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
      ///   @.str = private unnamed_addr constant [6 x i8] c"world\00", align 1

      auto TestSuiteNameConst = dyn_cast<GlobalValue>(TestSuiteNameConstRef->getOperand(0));
      assert(TestSuiteNameConst);

      auto TestCaseNameConst = dyn_cast<GlobalValue>(TestCaseNameConstRef->getOperand(0));
      assert(TestCaseNameConst);

      ///   [6 x i8] c"Hello\00"
      ///   [6 x i8] c"world\00"

      auto TestSuiteNameConstArray = dyn_cast<ConstantDataArray>(TestSuiteNameConst->getOperand(0));
      assert(TestSuiteNameConstArray);

      auto TestCaseNameConstArray = dyn_cast<ConstantDataArray>(TestCaseNameConst->getOperand(0));
      assert(TestCaseNameConstArray);

      ///   "Hello"
      ///   "world"

      auto TestSuiteName = TestSuiteNameConstArray->getRawDataValues().rtrim('\0');
      auto TestCaseName = TestCaseNameConstArray->getRawDataValues().rtrim('\0');

      /// Once we've got the Name of a Test Suite and the name of a Test Case
      /// We can construct the name of a Test
      auto TestName = TestSuiteName + "." + TestCaseName;

      /// And the part of Test Body function name

      auto TestBodyFunctionName = TestSuiteName + "_" + TestCaseName + "_Test8TestBodyEv";

      /// Using the TestBodyFunctionName we could find the function
      /// and finish creating the GoogleTest_Test object

      Function *TestBodyFunction = nullptr;
      for (auto &Func : M->getModule()->getFunctionList()) {
        auto foundPosition = Func.getName().rfind(StringRef(TestBodyFunctionName.str()));
        if (foundPosition != StringRef::npos) {
          TestBodyFunction = &Func;
          break;
        }
      }

      assert(TestBodyFunction && "Cannot find the TestBody function for the Test");

      tests.emplace_back(make_unique<GoogleTest_Test>(TestName.str(),
                                                      TestBodyFunction,
                                                      Ctx.getStaticConstructors()));
    }

  }

  return tests;
}

std::string demangle(const char* name) {

  int status = -4; // some arbitrary value to eliminate the compiler warning

  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void(*)(void*)> res {
    abi::__cxa_demangle(name, NULL, NULL, &status),
    std::free
  };

  return (status==0) ? res.get() : name ;
}

static bool shouldSkipDefinedFunction(llvm::Function *definedFunction) {
  if (definedFunction->getName().find(StringRef("testing8internal")) != StringRef::npos) {
    return true;
  }

  if (definedFunction->getName().find(StringRef("testing15AssertionResult")) != StringRef::npos) {
    return true;
  }

  if (definedFunction->getName().find(StringRef("testing7Message")) != StringRef::npos) {
    return true;
  }

  if (definedFunction->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = definedFunction->getMetadata(debugInfoKindID);
    DISubprogram *subprogram = dyn_cast<DISubprogram>(debug);
    if (subprogram) {
      if (subprogram->getFilename().contains("include/c++/v1")) {
        return true;
      }
    }
  }

  return false;
}

std::vector<Testee> GoogleTestFinder::findTestees(Test *Test,
                                                  Context &Ctx,
                                                  int maxDistance) {
  GoogleTest_Test *googleTest = dyn_cast<GoogleTest_Test>(Test);

  std::vector<Testee> testees;
  std::queue<Testee> traversees;
  std::set<Function *> checkedFunctions;

  Module *testBodyModule = googleTest->GetTestBodyFunction()->getParent();

  traversees.push(std::make_pair(googleTest->GetTestBodyFunction(), 0));

  while (true) {
    const Testee traversee = traversees.front();
    Function *traverseeFunction = traversee.first;
    const int mutationDistance = traversee.second;

    /// If the function we are processing is in the same translation unit
    /// as the test itself, then we are not looking for mutation points
    /// in this function assuming it to be a helper function, or the test itself
    if (traverseeFunction->getParent() != testBodyModule) {
      testees.push_back(traversee);
    }

    /// The function reached the max allowed distance
    /// Hence we don't go deeper
    if ( mutationDistance == maxDistance) {
      traversees.pop();
      if (traversees.size() == 0) {
        break;
      } else {
        continue;
      }
    }

    for (inst_iterator I = inst_begin(traverseeFunction),
         E = inst_end(traverseeFunction); I != E; ++I) {

      auto instruction = &*I;

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

      Function *definedFunction = testBodyModule->getFunction(functionOperand->getName());

      if (!definedFunction || definedFunction->isDeclaration()) {
        definedFunction = Ctx.lookupDefinedFunction(functionOperand->getName());
      }

      if (definedFunction) {
        auto functionWasNotProcessed = checkedFunctions.find(definedFunction) == checkedFunctions.end();
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
          traversees.push(std::make_pair(definedFunction, mutationDistance + 1));
        }
      }

    }

    traversees.pop();
    if (traversees.size() == 0) {
      break;
    }
  }

  return testees;
}

std::vector<MutationPoint *>
GoogleTestFinder::findMutationPoints(const Context &context,
                                     llvm::Function &testee) {

  if (MutationPointsRegistry.count(&testee) != 0) {
    return MutationPointsRegistry.at(&testee);
  }

  std::vector<MutationPoint *> points;

  GoogleTestMutationOperatorFilter filter;

  for (auto &mutationOperator : mutationOperators) {
    for (auto point : mutationOperator->getMutationPoints(context, &testee, filter)) {
      points.push_back(point);
      MutationPoints.emplace_back(std::unique_ptr<MutationPoint>(point));
    }
  }

  MutationPointsRegistry.insert(std::make_pair(&testee, points));
  return points;
}

std::vector<std::unique_ptr<MutationPoint>> GoogleTestFinder::findMutationPoints(
                             std::vector<MutationOperator *> &MutationOperators,
                             llvm::Function &F) {
  return std::vector<std::unique_ptr<MutationPoint>>();
}
