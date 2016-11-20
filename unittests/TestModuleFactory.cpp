
#include "TestModuleFactory.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SourceMgr.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

static LLVMContext GlobalCtx;

static inline bool fileExists(const std::string& name) {
  std::ifstream f(name.c_str());
  return f.good();
}

static std::string fixturePath(const char *fixtureName) {
  char cFixtureFullPath[256];

  getcwd(cFixtureFullPath, 255);
  strcat(cFixtureFullPath, "/fixtures/");
  strcat(cFixtureFullPath, fixtureName);

  std::string fixtureFullPath(cFixtureFullPath);

  return fixtureFullPath;
}

static std::string createFixture(const char *fixtureName) {
  std::string fixtureFullPath = fixturePath(fixtureName);

  assert(fileExists(fixtureFullPath));

  std::ifstream file(fixtureFullPath);
  std::string str;
  std::string file_contents;
  while (std::getline(file, str)) {
    file_contents += str;
    file_contents.push_back('\n');
  }

  return file_contents;
}

static std::unique_ptr<Module> parseIR(const char *IR) {
  SMDiagnostic Err;
  auto M = parseAssemblyString(IR, Err, GlobalCtx);

  /// FIXME: is there another way to check for errors?
  if (Err.getMessage().empty() == false) {
    Err.print("test", dbgs());
  }

  assert(M && "Expected module to be parsed correctly");

  assert(!llvm::verifyModule(*M, &dbgs()));

  return M;
}

std::unique_ptr<Module> TestModuleFactory::create_SimpleTest_NegateCondition_Tester_Module() {
  std::string contents = createFixture("fixture_simple_test_negate_condition_operator_tester.ll");

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier("fixture_simple_test_negate_condition_operator_tester.ll");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::create_SimpleTest_NegateCondition_Testee_Module() {
  std::string contents = createFixture("fixture_simple_test_negate_condition_operator_testee.ll");

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier("fixture_simple_test_negate_condition_operator_testee.ll");

  return module;
}

std::string TestModuleFactory::testerModulePath_IR() {
  return fixturePath("fixture_simple_test_tester_module.ll");
}

std::string TestModuleFactory::testerModulePath_Bitcode() {
  return fixturePath("fixture_simple_test_tester_module.bc");
}

std::unique_ptr<Module> TestModuleFactory::createTesterModule() {
  /// FIXME: Use testerModulePath() after merge with Stan's code
  std::string contents = createFixture("fixture_simple_test_tester_module.ll");

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier("fixture_simple_test_tester_module.ll");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createTesteeModule() {
  auto module = parseIR("define i32 @count_letters(i8* %s, i8 signext %c) {\n"
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

  module->setModuleIdentifier("count_letters");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createLibCTesterModule() {
  auto module = parseIR("declare i32 @sum(i32, i32)"
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

  module->setModuleIdentifier("sum");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createLibCTesteeModule() {
  auto module = parseIR("define i32 @sum(i32 %a, i32 %b) {\n"
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

  module->setModuleIdentifier("sum");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createExternalLibTesterModule() {
  auto module = parseIR("define i32 @test_in_memory_db() #0 {\n"
                        "entry:\n"
                        "  %call = call i32 (...) @in_memory_db()\n"
                        "  %cmp = icmp eq i32 %call, 0\n"
                        "  %conv = zext i1 %cmp to i32\n"
                        "  ret i32 %conv\n"
                        "}\n"
                        ""
                        "declare i32 @in_memory_db(...)\n");

  module->setModuleIdentifier("test_in_memory_db");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createExternalLibTesteeModule() {
  auto module = parseIR("%struct.sqlite3 = type opaque\n"
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

  module->setModuleIdentifier("in_memory_db");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createGoogleTestTesterModule() {
  auto module = parseIR("@.str = private unnamed_addr constant [6 x i8] c\"Hello\\00\", align 1 \n"
                        "@.str.1 = private unnamed_addr constant [6 x i8] c\"world\\00\", align 1 \n"

                        "%\"class.testing::internal::TestFactoryImpl\" = type opaque \n"
                        "%\"class.testing::internal::TestFactoryBase\" = type opaque \n"

                        "%\"class.testing::TestInfo\" = type opaque \n"

                        "%class.Hello_world_Test = type opaque \n"

                        "@_ZN14Hello_world_Test10test_info_E = global %\"class.testing::TestInfo\"* null, align 8 \n"

                        "define void @__cxx_global_var_init() { \n"
                        "entry: \n"
                        "  %call = call i8* @_ZN7testing8internal13GetTestTypeIdEv() \n"
                        "  %call1 = call i8* @_Znwm(i64 8) \n"
                        "  %0 = bitcast i8* %call1 to %\"class.testing::internal::TestFactoryImpl\"* \n"
                        "  call void @_ZN7testing8internal15TestFactoryImplI14Hello_sup_TestEC1Ev(%\"class.testing::internal::TestFactoryImpl\"* %0) \n"
                        "  %1 = bitcast %\"class.testing::internal::TestFactoryImpl\"* %0 to %\"class.testing::internal::TestFactoryBase\"*  \n"
                        "  %call2 = call %\"class.testing::TestInfo\"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i8* null, i8* null, i8* %call, void ()* @_ZN7testing4Test13SetUpTestCaseEv, void ()* @_ZN7testing4Test16TearDownTestCaseEv, %\"class.testing::internal::TestFactoryBase\"* %1) \n"
                        "  store %\"class.testing::TestInfo\"* %call2, %\"class.testing::TestInfo\"** @_ZN14Hello_world_Test10test_info_E, align 8 \n"
                        "  ret void \n"
                        "}\n"

                        "declare void @_ZN7testing4Test13SetUpTestCaseEv() \n"
                        "declare void @_ZN7testing4Test16TearDownTestCaseEv() \n"
                        "declare i8* @_ZN7testing8internal13GetTestTypeIdEv() \n"
                        "declare void @_ZN7testing8internal15TestFactoryImplI14Hello_sup_TestEC1Ev(%\"class.testing::internal::TestFactoryImpl\"* %this) \n"
                        "declare %\"class.testing::TestInfo\"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8*, i8*, i8*, i8*, i8*, void ()*, void ()*, %\"class.testing::internal::TestFactoryBase\"*) \n"
                        "declare i8* @_Znwm(i64) \n"

                        "declare void @_ZN16Hello_world_Test8TestBodyEv(%class.Hello_world_Test* %this) \n"
                        "");

  module->setModuleIdentifier("_ZN16Hello_world_Test8TestBodyEv");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::createGoogleTestTesteeModule() {
  auto module = parseIR("@.str = private unnamed_addr constant [6 x i8] c\"Hello\\00\", align 1 \n"
                        "@.str.1 = private unnamed_addr constant [6 x i8] c\"world\\00\", align 1 \n"

                        "%\"class.testing::internal::TestFactoryImpl\" = type opaque \n"
                        "%\"class.testing::internal::TestFactoryBase\" = type opaque \n"

                        "%\"class.testing::TestInfo\" = type opaque \n"

                        "%class.Hello_world_Test = type opaque \n"

                        "@_ZN14Hello_world_Test10test_info_E = global %\"class.testing::TestInfo\"* null, align 8 \n"

                        "define void @__cxx_global_var_init() { \n"
                        "entry: \n"
                        "  %call = call i8* @_ZN7testing8internal13GetTestTypeIdEv() \n"
                        "  %call1 = call i8* @_Znwm(i64 8) \n"
                        "  %0 = bitcast i8* %call1 to %\"class.testing::internal::TestFactoryImpl\"* \n"
                        "  call void @_ZN7testing8internal15TestFactoryImplI14Hello_sup_TestEC1Ev(%\"class.testing::internal::TestFactoryImpl\"* %0) \n"
                        "  %1 = bitcast %\"class.testing::internal::TestFactoryImpl\"* %0 to %\"class.testing::internal::TestFactoryBase\"*  \n"
                        "  %call2 = call %\"class.testing::TestInfo\"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0), i8* null, i8* null, i8* %call, void ()* @_ZN7testing4Test13SetUpTestCaseEv, void ()* @_ZN7testing4Test16TearDownTestCaseEv, %\"class.testing::internal::TestFactoryBase\"* %1) \n"
                        "  store %\"class.testing::TestInfo\"* %call2, %\"class.testing::TestInfo\"** @_ZN14Hello_world_Test10test_info_E, align 8 \n"
                        "  ret void \n"
                        "}\n"

                        "declare void @_ZN7testing4Test13SetUpTestCaseEv() \n"
                        "declare void @_ZN7testing4Test16TearDownTestCaseEv() \n"
                        "declare i8* @_ZN7testing8internal13GetTestTypeIdEv() \n"
                        "declare void @_ZN7testing8internal15TestFactoryImplI14Hello_sup_TestEC1Ev(%\"class.testing::internal::TestFactoryImpl\"* %this) \n"
                        "declare %\"class.testing::TestInfo\"* @_ZN7testing8internal23MakeAndRegisterTestInfoEPKcS2_S2_S2_PKvPFvvES6_PNS0_15TestFactoryBaseE(i8*, i8*, i8*, i8*, i8*, void ()*, void ()*, %\"class.testing::internal::TestFactoryBase\"*) \n"
                        "declare i8* @_Znwm(i64) \n"
                        
                        "declare void @_ZN16Hello_world_Test8TestBodyEv(%class.Hello_world_Test* %this) \n"
                        
                        "");

  module->setModuleIdentifier("_ZN16Hello_world_Test8TestBodyEv____x");
  
  return module;
}

std::unique_ptr<Module> TestModuleFactory::APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutation_operatorModule() {
  std::string contents = createFixture("APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutation_operator.ll");

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier("APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutation_operator.ll");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutation_operatorModule() {
  std::string contents = createFixture("APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutation_operator.ll");

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier("APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutation_operator.ll");

  return module;
}

std::unique_ptr<Module> TestModuleFactory::APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutation_operatorModule() {
  std::string contents = createFixture("APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutation_operator.ll");

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier("APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutation_operator.ll");

  return module;
}
