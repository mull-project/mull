
#include "TestModuleFactory.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SourceMgr.h"

static LLVMContext GlobalCtx;

static std::unique_ptr<Module> parseIR(const char *IR) {
    SMDiagnostic Err;
    auto M = parseAssemblyString(IR, Err, GlobalCtx);

    assert(M && "Expected module to be parsed correctly");

    /// FIXME: is there another way to check for errors?
    if (Err.getMessage().empty() == false) {
        Err.print("test", dbgs());
    }

    return M;
}

std::unique_ptr<Module> TestModuleFactory::createTesterModule() {
    auto module = parseIR("@.str = private unnamed_addr constant [6 x i8] c\"ababa\\00\", align 1 \n"
                          "define i32 @test_count_letters() { \n"
                          "entry: \n"
                          "  %call = call i32 @count_letters(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i8 signext 97) \n"
                          "  %cmp = icmp eq i32 %call, 3 \n"
                          "  %conv = zext i1 %cmp to i32 \n"
                          "  ret i32 %conv \n"
                          "} \n"
                          ""
                          "declare i32 @count_letters(i8*, i8 signext) \n");

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

    return module;
}

std::unique_ptr<Module> TestModuleFactory::createGoogleTestTesterModule() {
  auto module = parseIR("%\"class.(anonymous namespace)::APFloatTest_operatorOverloads_Test\" = type opaque"
                        "\n\n"

                        "%struct.anon.73 = type opaque"
                        "\n\n"

                        "define internal void "
                        "@_ZN12_GLOBAL__N_134APFloatTest_operatorOverloads_Test8TestBodyEv(%\"class.(anonymous namespace)::APFloatTest_operatorOverloads_Test\"*) {"
                        "ret void"
                        "}"
                        "\n\n"

                        "define internal void "
                        "@_ZN12_GLOBAL__N_134APFloatTest_operatorOverloads_Test8TestBodyEvEN3$_3D2Ev(%struct.anon.73*) {"
                        "ret void"
                        "}");

  return module;
}
