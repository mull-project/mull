#include "Context.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static std::unique_ptr<Module> parseIR(const char *IR) {
  LLVMContext &C = getGlobalContext();
  SMDiagnostic Err;
  return parseAssemblyString(IR, Err, C);
}

TEST(Context, AddModule) {
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

  ASSERT_EQ(1U, Ctx.getModules().size());
}
