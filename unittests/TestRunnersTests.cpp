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
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static LLVMContext GlobalCtx;

static std::unique_ptr<Module> parseIR(const char *IR) {
  SMDiagnostic Err;
  auto M = parseAssemblyString(IR, Err, GlobalCtx);

  /// FIXME: is there another way to check for errors?
  if (Err.getMessage().str().length() != 0) {
    Err.print("test", dbgs());
  }
  return M;
}

TEST(SimpleTestRunner, runTest) {
  auto OwnedModuleWithTests = parseIR("declare i32 @sum(i32, i32)\n"
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

  auto OwnedModuleWithTestees = parseIR("define i32 @sum(i32 %a, i32 %b) {\n"
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

  Module *ModuleWithTests = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  Context Ctx;
  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  ASSERT_NE(0U, Tests.size());

  Function *Test = *(Tests.begin());

  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests);

  SimpleTestRunner Runner(Modules);

  /// Here we run test with original testee function
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees));

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);
  ASSERT_NE(0U, Testees.size());
  Function *Testee = *(Testees.begin());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  MutationEngine Engine;
  Engine.applyMutation(*MP);

  ASSERT_EQ(Failed, Runner.runTest(Test, ModuleWithTestees));
}

TEST(SimpleTestRunner, runTestUsingLibC) {
  auto OwnedModuleWithTests = parseIR("@.str = private unnamed_addr constant [6 x i8] c\"ababa\\00\", align 1 \n"
                                      "define i32 @test_count_letters() { \n"
                                      "entry: \n"
                                      "  %call = call i32 @count_letters(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8 signext 97) \n"
                                      "  %cmp = icmp eq i32 %call, 3 \n"
                                      "  %conv = zext i1 %cmp to i32 \n"
                                      "  ret i32 %conv \n"
                                      "} \n"
                                      ""
                                      "declare i32 @count_letters(i8*, i8 signext) \n");

  auto OwnedModuleWithTestees = parseIR("define i32 @count_letters(i8* %s, i8 signext %c) {\n"
                                        "entry:\n"
                                        "  %s.addr = alloca i8*, align 8\n"
                                        "  %c.addr = alloca i8, align 1\n"
                                        "  %count = alloca i32, align 4\n"
                                        "  %found = alloca i8*, align 8\n"
                                        "  store i8* %s, i8** %s.addr, align 8\n"
                                        "  store i8 %c, i8* %c.addr, align 1\n"
                                        "  store i32 0, i32* %count, align 4\n"
                                        "  %0 = load i8*, i8** %s.addr, align 8\n"
                                        "  store i8* %0, i8** %found, align 8\n"
                                        "  br label %while.cond\n"
                                        ""
                                        "while.cond: ; preds = %while.body, %entry\n"
                                        "  %1 = load i8*, i8** %found, align 8\n"
                                        "  %2 = load i8, i8* %c.addr, align 1\n"
                                        "  %conv = sext i8 %2 to i32\n"
                                        "  %call = call i8* @strchr(i8* %1, i32 %conv)\n"
                                        "  store i8* %call, i8** %found, align 8\n"
                                        "  %tobool = icmp ne i8* %call, null\n"
                                        "  br i1 %tobool, label %while.body, label %while.end\n"
                                        ""
                                        "while.body: ; preds = %while.cond\n"
                                        "  %3 = load i32, i32* %count, align 4\n"
                                        "  %add = add nsw i32 %3, 1\n"
                                        "  store i32 %add, i32* %count, align 4\n"
                                        "  %4 = load i8*, i8** %found, align 8\n"
                                        "  %add.ptr = getelementptr inbounds i8, i8* %4, i64 1\n"
                                        "  store i8* %add.ptr, i8** %found, align 8\n"
                                        "  br label %while.cond\n"
                                        ""
                                        "while.end:  ; preds = %while.cond\n"
                                        "  %5 = load i32, i32* %count, align 4\n"
                                        "  ret i32 %5\n"
                                        "}\n"
                                        ""
                                        "declare i8* @strchr(i8*, i32)");

  Module *ModuleWithTests = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  Context Ctx;
  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();

  ASSERT_NE(0U, Tests.size());

  Function *Test = *(Tests.begin());

  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests);

  SimpleTestRunner Runner(Modules);

  /// Here we run test with original testee function
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees));

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  ArrayRef<Function *> Testees = Finder.findTestees(*Test);
  ASSERT_NE(0U, Testees.size());
  Function *Testee = *(Testees.begin());

  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutOps({&MutOp});

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints = Finder.findMutationPoints(MutOps, *Testee);

  MutationPoint *MP = (*(MutationPoints.begin())).get();
  MutationEngine Engine;
  Engine.applyMutation(*MP);

  ASSERT_EQ(Failed, Runner.runTest(Test, ModuleWithTestees));
}

TEST(SimpleTestRunner, runTestUsingExternalLibrary) {
  /// No mutations applied here, since the only point of interest
  /// is an external libraries, in this case it is 'sqlite3'
  auto OwnedModuleWithTests = parseIR("define i32 @test_in_memory_db() #0 {\n"
                                      "entry:\n"
                                      "  %call = call i32 (...) @in_memory_db()\n"
                                      "  %cmp = icmp eq i32 %call, 0\n"
                                      "  %conv = zext i1 %cmp to i32\n"
                                      "  ret i32 %conv\n"
                                      "}\n"
                                      ""
                                      "declare i32 @in_memory_db(...)\n");

  auto OwnedModuleWithTestees = parseIR("%struct.sqlite3 = type opaque\n"
                                        ""
                                        "@.str = private unnamed_addr constant [9 x i8] c\":memory:\\00\", align 1\n"
                                        ""
                                        "define i32 @in_memory_db() #0 {\n"
                                        "entry:\n"
                                        "  %db = alloca %struct.sqlite3*, align 8\n"
                                        "  %call = call i32 @sqlite3_open(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), %struct.sqlite3** %db)\n"
                                        "  ret i32 %call\n"
                                        "}\n"
                                        ""
                                        "declare i32 @sqlite3_open(i8*, %struct.sqlite3**) #1\n");

  Module *ModuleWithTests = OwnedModuleWithTests.get();
  Module *ModuleWithTestees = OwnedModuleWithTestees.get();

  Context Ctx;
  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));

  SimpleTestFinder Finder(Ctx);
  ArrayRef<Function *> Tests = Finder.findTests();
  
  ASSERT_NE(0U, Tests.size());
  
  Function *Test = *(Tests.begin());
  
  std::vector<Module *> Modules;
  Modules.push_back(ModuleWithTests);
  
  SimpleTestRunner Runner(Modules);
  
  llvm::sys::DynamicLibrary::LoadLibraryPermanently("/usr/lib/libsqlite3.dylib");
  
  ASSERT_EQ(Passed, Runner.runTest(Test, ModuleWithTestees));
}
