#include "Rust/RustTestFinder.h"

#include "Config.h"
#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "TestModuleFactory.h"
#include "Test.h"
#include "Toolchain/Toolchain.h"
#include "Rust/RustTest.h"
#include "Rust/RustTestRunner.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(RustTestFinder, FindTest) {
  auto rustModule = TestModuleFactory.rustModule();
  auto mullRustModule = make_unique<MullModule>(std::move(rustModule), "");

  Context Ctx;
  Ctx.addModule(std::move(mullRustModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());
  RustTestFinder finder(std::move(mutationOperators));

  auto tests = finder.findTests(Ctx);

  ASSERT_EQ(4U, tests.size());
}

TEST(RustTestFinder, FindTestee) {
  auto rustModule = TestModuleFactory.rustModule();
  auto mullRustModule = make_unique<MullModule>(std::move(rustModule), "");

  Context Ctx;
  Ctx.addModule(std::move(mullRustModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());
  RustTestFinder finder(std::move(mutationOperators));

  auto Tests = finder.findTests(Ctx);

  auto &Test = *(Tests.begin());

  std::vector<std::unique_ptr<Testee>> Testees = finder.findTestees(Test.get(), Ctx, 4);

  ASSERT_EQ(2U, Testees.size());

//  Function *Testee = Testees[1]->getTesteeFunction();
//  ASSERT_FALSE(Testee->empty());
}
