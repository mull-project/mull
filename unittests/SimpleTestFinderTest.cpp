#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
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

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));

  SimpleTestFinder finder(Ctx);

  auto tests = finder.findTests();

  ASSERT_EQ(1U, tests.size());
}

TEST(SimpleTestFinder, FindTestee) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  auto Tests = Finder.findTests();

  auto &Test = *(Tests.begin());

  ArrayRef<Function *> Testees = Finder.findTestees(Test.get());

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = *(Testees.begin());
  ASSERT_FALSE(Testee->empty());
}

TEST(SimpleTestFinder, FindMutationPoints) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  Context Ctx;
  Ctx.addModule(std::move(ModuleWithTests));
  Ctx.addModule(std::move(ModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  auto Tests = Finder.findTests();

  auto &Test = *Tests.begin();

  ArrayRef<Function *> Testees = Finder.findTestees(Test.get());

  ASSERT_EQ(1U, Testees.size());

  Function *Testee = *(Testees.begin());
  ASSERT_FALSE(Testee->empty());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  ASSERT_EQ(&MutOp, MP->getOperator());
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 2);
  ASSERT_TRUE(MPA.getIIndex() == 1);
}
