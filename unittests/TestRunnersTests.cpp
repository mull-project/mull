#include "TestFinders/SimpleTestFinder.h"
#include "TestRunners/SimpleTestRunner.h"
#include "MutationEngine.h"
#include "Context.h"
#include "MutationOperators/AddMutationOperator.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static LLVMContext GlobalCtx;

static std::unique_ptr<Module> parseIR(const char *IR) {
  SMDiagnostic Err;
  return parseAssemblyString(IR, Err, GlobalCtx);
}

TEST(SimpleTestRunner, runTest) {
  auto ModuleWithTests = parseIR("declare i32 @sum(i32, i32)\n"
                                 "define i32 @test_main() {\n"
                                 "entry:\n"
                                 "  %result = alloca i32, align 4\n"
                                 "  %result_matches = alloca i32, align 4\n"
                                 "  %call = call i32 @sum(i32 3, i32 5)\n"
                                 "  store i32 %call, i32* %result, align 4\n"
                                 "  %0 = load i32, i32* %result, align 4\n"
                                 "  %cmp = icmp eq i32 %0, 8\n"
                                 "  %conv = zext i1 %cmp to i32\n"
                                 "  store i32 %conv, i32* %result_matches, align 4\n"
                                 "  %1 = load i32, i32* %result_matches, align 4\n"
                                 "  ret i32 %1\n"
                                 "}\n");

  auto ModuleWithTestees = parseIR("define i32 @sum(i32 %a, i32 %b) {\n"
                                   "entry:\n"
                                   "  %a.addr = alloca i32, align 4\n"
                                   "  %b.addr = alloca i32, align 4\n"
                                   "  store i32 %a, i32* %a.addr, align 4\n"
                                   "  store i32 %b, i32* %b.addr, align 4\n"
                                   "  %0 = load i32, i32* %a.addr, align 4\n"
                                   "  %1 = load i32, i32* %b.addr, align 4\n"
                                   "  %add = add nsw i32 %0, %1\n"
                                   "  ret i32 %add\n"
                                   "}");

  Context Ctx;
  Ctx.addModule(ModuleWithTests.get());
  Ctx.addModule(ModuleWithTestees.get());

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  ASSERT_NE(0U, Tests.size());

  Function *Test = *(Tests.begin());

  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests.get());

  SimpleTestRunner Runner(Modules);

  /// Here we run test with original testee function
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees.get()));

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);
  ASSERT_NE(0U, Testees.size());
  Function *Testee = *(Testees.begin());

  AddMutationOperator MutOp;
  ArrayRef<MutationOperator *> MutOps(&MutOp);

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  MutationEngine Engine;
  Engine.applyMutation(*MP);

  ASSERT_EQ(Failed, Runner.runTest(Test, ModuleWithTestees.get()));
}
