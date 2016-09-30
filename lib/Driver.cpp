#include "Driver.h"
#include "Compiler.h"
#include "Config.h"
#include "Context.h"
#include "ModuleLoader.h"
#include "TestResult.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "TestFinder.h"
#include "TestRunner.h"

/// FIXME: Should be abstract
#include "MutationOperators/AddMutationOperator.h"

#include <algorithm>

using namespace llvm;
using namespace Mutang;

/// Populate Mutang::Context with modules using
/// ModulePaths from Mutang::Config.
/// Mutang::Context should be populated using ModuleLoader
/// so that we could inject modules from string for testing purposes

/// Having Mutang::Context in place we could instantiate TestFinder and find all tests
/// Using same TestFinder we could find mutation points, apply them sequentially and
/// run tests/mutants using newly created TestRunner

/// This method should return (somehow) results of the tests/mutants execution
/// So that we could easily plug in some TestReporter

/// UPD: The method returns set of results
/// Number of results equals to a number of tests
/// Each result contains result of execution of an original test and
/// all the results of each mutant within corresponding MutationPoint

std::vector<std::unique_ptr<TestResult>> Driver::Run() {
  Compiler Compiler;

  std::vector<std::unique_ptr<TestResult>> Results;

  /// Assumption: all modules will be used during the execution
  /// Therefore we load them into memory and compile immediately
  /// Later on modules used only for generating of mutants
  for (auto ModulePath : Cfg.GetBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
    assert(OwnedModule && "Can't load module");

    auto Module = OwnedModule.get();
    auto ObjectFile = Compiler.CompilerModule(Module);
    InnerCache.insert(std::make_pair(Module, std::move(ObjectFile)));

    Ctx.addModule(std::move(OwnedModule));
  }

  /// FIXME: Should come from the outside
  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutationOperators;
  MutationOperators.push_back(&MutOp);

  for (auto &Test : Finder.findTests(Ctx)) {
    auto ObjectFiles = AllObjectFiles();

    ExecutionResult ExecResult = Sandbox->run([&](ExecutionResult *SharedResult){
      *SharedResult = Runner.runTest(Test.get(), ObjectFiles);
    });

    auto BorrowedTest = Test.get();
    auto Result = make_unique<TestResult>(ExecResult, std::move(Test));

    for (auto Testee : Finder.findTestees(BorrowedTest, Ctx)) {
      auto ObjectFiles = AllButOne(Testee->getParent());
      for (auto &MutationPoint : Finder.findMutationPoints(MutationOperators, *Testee)) {

        Module *TesteeModuleCopy = CloneModule(Testee->getParent()).release();

        ExecutionResult R = Sandbox->run([&](ExecutionResult *SharedResult){
          MutationPoint->applyMutation(TesteeModuleCopy);

          auto Mutant = Compiler.CompilerModule(TesteeModuleCopy);
          ObjectFiles.push_back(Mutant.getBinary());

          *SharedResult = Runner.runTest(BorrowedTest, ObjectFiles);

          assert(SharedResult->Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
        });

        assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

        auto MutResult = make_unique<MutationResult>(R, std::move(MutationPoint));
        Result->addMutantResult(std::move(MutResult));
      }
    }

    Results.push_back(std::move(Result));
  }

  return Results;
}

std::vector<llvm::object::ObjectFile *> Driver::AllButOne(llvm::Module *One) {
  std::vector<llvm::object::ObjectFile *> Objects;

  for (auto &CachedEntry : InnerCache) {
    if (One != CachedEntry.first) {
      Objects.push_back(CachedEntry.second.getBinary());
    }
  }

  return Objects;
}

std::vector<llvm::object::ObjectFile *> Driver::AllObjectFiles() {
  std::vector<llvm::object::ObjectFile *> Objects;

  for (auto &CachedEntry : InnerCache) {
    Objects.push_back(CachedEntry.second.getBinary());
  }

  return Objects;
}

#pragma mark - Debug

void Driver::debug_PrintTestNames() {
  for (auto ModulePath : Cfg.GetBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
    assert(OwnedModule && "Can't load module");
    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    printf("%s\n", Test->getTestName().c_str());
  }
}

void Driver::debug_PrintTesteeNames() {
  for (auto ModulePath : Cfg.GetBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
    assert(OwnedModule && "Can't load module");
    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    printf("%s\n", Test->getTestName().c_str());
    for (auto &Testee : Finder.findTestees(Test.get(), Ctx)) {
//      auto MD = Testee->getMetadata(LLVMContext::MD_dbg);
//      auto Dbg = dyn_cast<DISubprogram>(MD);
//      if (Dbg) {
//        printf("\t%s\n", Dbg->getFilename().str().c_str());
//      }
      printf("\t%s\n", Testee->getName().str().c_str());
    }
  }
}
