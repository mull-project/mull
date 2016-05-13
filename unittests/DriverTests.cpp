#include "Driver.h"
#include "Config.h"
#include "Context.h"
#include "ModuleLoader.h"
#include "TestFinders/SimpleTestFinder.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

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

class FakeModuleLoader : public ModuleLoader {
public:
  FakeModuleLoader() : ModuleLoader(GlobalCtx) {}

  std::unique_ptr<llvm::Module> loadModuleAtPath(const std::string &path) override {
    if (path == "foo") {
      return parseIR("declare i32 @sum(i32, i32)"
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
    } else if (path == "bar") {
      return parseIR("define i32 @sum(i32 %a, i32 %b) {\n"
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
    }

    return nullptr;
  }
};

TEST(Driver, SimpleTest) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  std::vector<std::string> ModulePaths({ "foo", "bar" });
  Config Cfg(ModulePaths);

  FakeModuleLoader Loader;

  Driver Driver(Cfg, Loader);
  Driver.Run();
}
