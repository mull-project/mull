#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"

#include "TestModuleFactory.h"
#include "Test.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestFinder, FindTest) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto mutangModuleWithTests = make_unique<MutangModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder finder(std::move(mutationOperators));

  auto tests = finder.findTests(Ctx);

  ASSERT_EQ(1U, tests.size());
}

TEST(SimpleTestFinder, FindTestee) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mutangModuleWithTests = make_unique<MutangModule>(std::move(ModuleWithTests), "");
  auto mutangModuleWithTestees = make_unique<MutangModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));
  Ctx.addModule(std::move(mutangModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  auto Tests = Finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  ArrayRef<Testee> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = (Testees.begin())->first;
  ASSERT_FALSE(Testee->empty());
}

TEST(SimpleTestFinder, FindMutationPoints_AddMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mutangModuleWithTests   = make_unique<MutangModule>(std::move(ModuleWithTests), "");
  auto mutangModuleWithTestees = make_unique<MutangModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));
  Ctx.addModule(std::move(mutangModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::unique_ptr<AddMutationOperator> addMutationOperator = make_unique<AddMutationOperator>();
  mutationOperators.emplace_back(std::move(addMutationOperator));

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *Tests.begin();

  ArrayRef<Testee> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = (Testees.begin())->first;
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

TEST(SimpleTestFinder, FindMutationPoints_NegateConditionMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();

  auto mutangModuleWithTests   = make_unique<MutangModule>(std::move(ModuleWithTests), "");
  auto mutangModuleWithTestees = make_unique<MutangModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));
  Ctx.addModule(std::move(mutangModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  auto Tests = Finder.findTests(Ctx);

  auto &Test = *Tests.begin();

  ArrayRef<Testee> Testees = Finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = (Testees.begin())->first;
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
