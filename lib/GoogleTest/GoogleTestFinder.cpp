#include "GoogleTest/GoogleTestFinder.h"

#include "Context.h"
#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include "GoogleTest/GoogleTest_Test.h"

#include <vector>

using namespace Mutang;
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

std::vector<std::unique_ptr<Test>> GoogleTestFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &M : Ctx.getModules()) {
    for (auto &Global : M->getGlobalList()) {
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
      ///   1. i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0)
      ///   2. @.str = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
      ///   3. [6 x i8] c"Hello\00"
      ///   4. "Hello"

      /// 1
      auto TestSuiteNameConstRef = dyn_cast<ConstantExpr>(CallInstruction->getArgOperand(0));
      assert(TestSuiteNameConstRef);

      /// 2
      auto TestSuiteNameConst = dyn_cast<GlobalValue>(TestSuiteNameConstRef->getOperand(0));
      assert(TestSuiteNameConst);

      /// 3
      auto TestSuiteNameConstArray = dyn_cast<ConstantDataArray>(TestSuiteNameConst->getOperand(0));
      assert(TestSuiteNameConstArray);

      /// 4
      auto TestSuiteName = TestSuiteNameConstArray->getRawDataValues();

      /// 1
      auto TestCaseNameConstRef = dyn_cast<ConstantExpr>(CallInstruction->getArgOperand(1));
      assert(TestCaseNameConstRef);

      /// 2
      auto TestCaseNameConst = dyn_cast<GlobalValue>(TestCaseNameConstRef->getOperand(0));
      assert(TestCaseNameConst);

      /// 3
      auto TestCaseNameConstArray = dyn_cast<ConstantDataArray>(TestCaseNameConst->getOperand(0));
      assert(TestCaseNameConstArray);

      /// 4
      auto TestCaseName = TestCaseNameConstArray->getRawDataValues();

      auto TestName = TestSuiteName.rtrim('\0') + "." + TestCaseName.rtrim('\0');

      tests.emplace_back(make_unique<GoogleTest_Test>(TestName.str(), Ctx.getStaticConstructors()));
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
