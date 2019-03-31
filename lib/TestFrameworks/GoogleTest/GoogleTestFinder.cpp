#include "mull/TestFrameworks/GoogleTest/GoogleTestFinder.h"

#include "mull/Filter.h"
#include "mull/Logger.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/GoogleTest/GoogleTest_Test.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

#include <vector>

using namespace mull;
using namespace llvm;

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

std::vector<std::unique_ptr<Test>> GoogleTestFinder::findTests(Program &program,
                                                               Filter &filter) {
  std::vector<std::unique_ptr<Test>> tests;

  auto testInfoTypeName = StringRef("class.testing::TestInfo");

  for (auto &currentModule : program.modules()) {
    for (auto &globalValue : currentModule->getModule()->getGlobalList()) {
      Type *globalValueType = globalValue.getValueType();
      if (globalValueType->getTypeID() != Type::PointerTyID) {
        continue;
      }

      Type *globalType = nullptr;
      if (globalValueType->getTypeID() == Type::PointerTyID) {
        globalType = globalValueType->getPointerElementType();
      } else {
        globalType = globalValueType->getSequentialElementType();
      }

      if (!globalType) {
        continue;
      }

      StructType *structType = dyn_cast<StructType>(globalType);
      if (!structType) {
        continue;
      }

      /// If two modules contain the same type, then when second modules is
      /// loaded the typename is changed a bit, e.g.:
      ///
      ///   class.testing::TestInfo     // type from first module
      ///   class.testing::TestInfo.25  // type from second module
      ///
      /// Hence we cannot just compare string, and rather should
      /// compare the beginning of the typename

      if (!structType->getName().startswith(testInfoTypeName)) {
        continue;
      }

      /// Normally the globalValue has only one usage, ut LLVM could add
      /// intrinsics such as @llvm.invariant.start
      /// We need to find a user that is a store instruction, which is
      /// a part of initialization function
      /// It looks like this:
      ///
      ///   store %"class.testing::TestInfo"* %call2,
      ///   %"class.testing::TestInfo"** @_ZN16Hello_world_Test10test_info_E
      ///
      /// From here we need to extract actual user, which is a `store`
      /// instruction The `store` instruction uses variable `%call2`, which is
      /// created from the following code:
      ///
      ///   %call2 = call %"class.testing::TestInfo"*
      ///   @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8*
      ///   getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0),
      ///   i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.1, i32 0, i32
      ///   0), i8* null, i8* null, i8* %call, void ()*
      ///   @_ZN7testing4Test13SetUpTestCaseEv, void ()*
      ///   @_ZN7testing4Test16TearDownTestCaseEv,
      ///   %"class.testing::internal::TestFactoryBase"* %1)
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

      StoreInst *storeInstruction = nullptr;
      for (auto userIterator = globalValue.user_begin();
           userIterator != globalValue.user_end(); userIterator++) {
        auto user = *userIterator;
        if (isa<StoreInst>(user)) {
          storeInstruction = dyn_cast<StoreInst>(user);
          break;
        }
      }

      assert(storeInstruction &&
             "The Global should be used within a store instruction");
      auto valueOperand = storeInstruction->getValueOperand();

      auto callSite = CallSite(valueOperand);
      assert((callSite.isCall() || callSite.isInvoke()) &&
             "Store should be using call to MakeAndRegisterTestInfo");

      /// Once we have the CallInstruction we can extract Test Suite Name
      /// and Test Case Name
      /// To extract them we need climb to the top, i.e.:
      ///
      ///   i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0)
      ///   i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str1, i32 0, i32
      ///   0)

      auto testSuiteNameConstRef =
          dyn_cast<ConstantExpr>(callSite->getOperand(0));
      assert(testSuiteNameConstRef);

      auto testCaseNameConstRef =
          dyn_cast<ConstantExpr>(callSite->getOperand(1));
      assert(testCaseNameConstRef);

      ///   @.str = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
      ///   @.str = private unnamed_addr constant [6 x i8] c"world\00", align 1

      auto testSuiteNameConst =
          dyn_cast<GlobalValue>(testSuiteNameConstRef->getOperand(0));
      assert(testSuiteNameConst);

      auto testCaseNameConst =
          dyn_cast<GlobalValue>(testCaseNameConstRef->getOperand(0));
      assert(testCaseNameConst);

      ///   [6 x i8] c"Hello\00"
      ///   [6 x i8] c"world\00"

      auto testSuiteNameConstArray =
          dyn_cast<ConstantDataArray>(testSuiteNameConst->getOperand(0));
      assert(testSuiteNameConstArray);

      auto testCaseNameConstArray =
          dyn_cast<ConstantDataArray>(testCaseNameConst->getOperand(0));
      assert(testCaseNameConstArray);

      ///   "Hello"
      ///   "world"

      std::string testSuiteName =
          testSuiteNameConstArray->getRawDataValues().rtrim('\0').str();
      std::string testCaseName =
          testCaseNameConstArray->getRawDataValues().rtrim('\0').str();

      /// Once we've got the Name of a Test Suite and the name of a Test Case
      /// We can construct the name of a Test
      const std::string testName = testSuiteName + "." + testCaseName;
      if (filter.shouldSkipTest(testName)) {
        continue;
      }

      /// And the part of Test Body function name
      std::string testBodyFunctionName =
          testSuiteName + "_" + testCaseName + "_Test8TestBodyEv";
      StringRef testBodyFunctionNameRef(testBodyFunctionName);

      /// Using the TestBodyFunctionName we could find the function
      /// and finish creating the GoogleTest_Test object

      Function *testBodyFunction = nullptr;
      for (auto &func : currentModule->getModule()->getFunctionList()) {
        auto foundPosition = func.getName().rfind(testBodyFunctionNameRef);
        if (foundPosition != StringRef::npos) {
          testBodyFunction = &func;
          break;
        }
      }

      assert(testBodyFunction &&
             "Cannot find the TestBody function for the Test");

      tests.emplace_back(make_unique<GoogleTest_Test>(
          testName, testBodyFunction, program.getStaticConstructors()));
    }
  }

  return tests;
}
