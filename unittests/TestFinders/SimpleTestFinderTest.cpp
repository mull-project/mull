#include "TestFinders/SimpleTestFinder.h"
#include "Context.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static LLVMContext Ctx;

std::unique_ptr<Module> parseIR(const char *IR) {
  SMDiagnostic Err;
  return parseAssemblyString(IR, Err, Ctx);
}

TEST(SimpleTestFinder, FindTest) {
  auto ModuleWithTests = parseIR("declare i32 @sum(i32, i32)"
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
  Context Ctx;
  Ctx.addModule(ModuleWithTests.get());

  SimpleTestFinder finder(Ctx);

  ArrayRef<Function *> tests = finder.findTests();

  EXPECT_EQ(tests.size(), 1);
}

TEST(SimpleTestFinder, FindTestee) {
  auto ModuleWithTests = parseIR("declare i32 @sum(i32, i32)"
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

  Function *Test = *(Tests.begin());

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);

  EXPECT_EQ(Testees.size(), 1);

  Function *Testee = *(Testees.begin());
  EXPECT_EQ(Testee->empty(), false);
}
