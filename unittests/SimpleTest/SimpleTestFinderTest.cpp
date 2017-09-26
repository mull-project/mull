#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"

#include "TestModuleFactory.h"
#include "Test.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestFinder, FindTest) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto mullModuleWithTests = make_unique<MullModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder finder(std::move(mutationOperators));

  auto tests = finder.findTests(Ctx);

  ASSERT_EQ(1U, tests.size());
}

TEST(SimpleTestFinder, FindTestee) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mullModuleWithTests = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto Tests = Finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

  Function *Testee = Testees[1]->getTesteeFunction();
  ASSERT_FALSE(Testee->empty());
}

TEST(SimpleTestFinder, FindMutationPoints_AddMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::unique_ptr<AddMutationOperator> addMutationOperator = make_unique<AddMutationOperator>();
  mutationOperators.emplace_back(std::move(addMutationOperator));

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *Tests.begin();

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

  Function *Testee = Testees[1]->getTesteeFunction();
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  /// TODO: Don't know how to compare unique pointer addMutationOperator with
  /// MutationOperator *.
  //ASSERT_EQ(addMutationOperator.get(), MP->getOperator());
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 2);
  ASSERT_TRUE(MPA.getIIndex() == 1);
}

TEST(SimpleTestFinder, FindMutationPoints_MathSubMutationOperator) {
  auto module = TestModuleFactory.create_SimpleTest_MathSub_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathSubMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *Tests.begin();

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

  Function *Testee = Testees[1]->getTesteeFunction();
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 6);
}

TEST(SimpleTestFinder, FindMutationPoints_NegateConditionMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *Tests.begin();

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

  Function *Testee = Testees[1]->getTesteeFunction();
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  /// TODO: Don't know how to compare unique pointer addMutationOperator with
  /// MutationOperator *.
  //ASSERT_EQ(addMutationOperator.get(), MP->getOperator());
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 7);
}

TEST(SimpleTestFinder, FindMutationPoints_RemoteVoidFunctionMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_RemoveVoidFunction_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_RemoveVoidFunction_Testee_Module();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *Tests.begin();

  std::vector<std::unique_ptr<Testee>> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(3U, Testees.size());

  Function *Testee2 = Testees[1]->getTesteeFunction(); // testee()
  Function *Testee3 = Testees[2]->getTesteeFunction(); // void_function()

  ASSERT_FALSE(Testee2->empty());
  ASSERT_FALSE(Testee3->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee2);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  /// TODO: Don't know how to compare unique pointer addMutationOperator with
  /// MutationOperator *.
  //ASSERT_EQ(addMutationOperator.get(), MP->getOperator());
  ASSERT_TRUE(isa<CallInst>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 2);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 2);
}

TEST(SimpleTestFinder, findMutationPoints_AndOrReplacementMutationOperator) {
  auto module = TestModuleFactory.create_SimpleTest_ANDORReplacement_Module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context ctx;
  ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto tests = Finder.findTests(ctx);
  ASSERT_EQ(8U, tests.size());

  /// Test #1
  auto &test = *tests.begin();

  std::vector<std::unique_ptr<Testee>> testees = Finder.findTestees(test.get(), ctx, 4);

  ASSERT_EQ(2U, testees.size());

  Function *testee_test_and_operator = testees[1]->getTesteeFunction(); // testee_and_operator()

  std::vector<MutationPoint *> MutationPoints =
    Finder.findMutationPoints(ctx, *testee_test_and_operator);

  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *mutationPoint = (*(MutationPoints.begin()));

  MutationPointAddress mutationPointAddress = mutationPoint->getAddress();
  ASSERT_EQ(mutationPointAddress.getFnIndex(), 0);
  ASSERT_EQ(mutationPointAddress.getBBIndex(), 0);
  ASSERT_EQ(mutationPointAddress.getIIndex(), 10);

  /// Test #5
  auto &test5 = tests[4];

  std::vector<std::unique_ptr<Testee>> test5_testees = Finder.findTestees(test5.get(), ctx, 4);

  ASSERT_EQ(2U, test5_testees.size());

  Function *testee5_function = test5_testees[1]->getTesteeFunction(); // testee_and_operator()

  std::vector<MutationPoint *> testee5_mutationPoints =
    Finder.findMutationPoints(ctx, *testee5_function);

  ASSERT_EQ(2U, testee5_mutationPoints.size());

  MutationPoint *testee5_mutationPoint1 = testee5_mutationPoints[0];

  MutationPointAddress testee5_mutationPoint1_address = testee5_mutationPoint1->getAddress();
  ASSERT_EQ(testee5_mutationPoint1_address.getFnIndex(), 5);
  ASSERT_EQ(testee5_mutationPoint1_address.getBBIndex(), 0);
  ASSERT_EQ(testee5_mutationPoint1_address.getIIndex(), 10);

  MutationPoint *testee5_mutationPoint2 = testee5_mutationPoints[1];

  MutationPointAddress testee5_mutationPoint2_address = testee5_mutationPoint2->getAddress();
  ASSERT_EQ(testee5_mutationPoint2_address.getFnIndex(), 5);
  ASSERT_EQ(testee5_mutationPoint2_address.getBBIndex(), 1);
  ASSERT_EQ(testee5_mutationPoint2_address.getIIndex(), 3);
}

TEST(SimpleTestFinder, FindTestees_TesteePathMemorization) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_testeePathCalculation_tester();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_testeePathCalculation_testee();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);
  ASSERT_EQ(1U, Tests.size());

  SimpleTest_Test *test = dyn_cast<SimpleTest_Test>((*Tests.begin()).get());

  std::vector<std::unique_ptr<Testee>> testees = Finder.findTestees(test, Ctx, 4);

  ASSERT_EQ(5U, testees.size());

  Testee *testee1 = testees[0].get();
  Testee *testee2 = testees[1].get();
  Testee *testee3 = testees[2].get();
  Testee *testee4 = testees[3].get();
  Testee *testee5 = testees[4].get();

  Function *testeeFunction1 = testee1->getTesteeFunction();
  Function *testeeFunction2 = testee2->getTesteeFunction();
  Function *testeeFunction3 = testee3->getTesteeFunction();
  Function *testeeFunction4 = testee4->getTesteeFunction();
  Function *testeeFunction5 = testee5->getTesteeFunction();

  ASSERT_FALSE(testeeFunction1->empty());
  ASSERT_FALSE(testeeFunction2->empty());
  ASSERT_FALSE(testeeFunction3->empty());
  ASSERT_FALSE(testeeFunction4->empty());
  ASSERT_FALSE(testeeFunction5->empty());

  ASSERT_EQ(testeeFunction1->getName(), "test_testee_path_calculation");
  ASSERT_EQ(testeeFunction2->getName(), "testee1");
  ASSERT_EQ(testeeFunction3->getName(), "testee2");
  ASSERT_EQ(testeeFunction4->getName(), "testee3");
  ASSERT_EQ(testeeFunction5->getName(), "testee4");

  ASSERT_EQ(testee1->getCallerTestee(), nullptr);
  ASSERT_EQ(testee2->getCallerTestee(), testee1);
  ASSERT_EQ(testee3->getCallerTestee(), testee2);
  ASSERT_EQ(testee4->getCallerTestee(), testee3);
  ASSERT_EQ(testee5->getCallerTestee(), testee4);
}
