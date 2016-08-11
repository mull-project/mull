#include "Driver.h"
#include "Compiler.h"
#include "Config.h"
#include "Context.h"
#include "ModuleLoader.h"
#include "TestResult.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

/// FIXME: Should be abstract
#include "TestFinders/SimpleTestFinder.h"
#include "TestFinders/GoogleTestFinder.h"

/// FIXME: Should be abstract
#include "TestRunners/SimpleTestRunner.h"
#include "TestRunners/GoogleTestRunner.h"

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

  SimpleTestRunner Runner;

  SimpleTestFinder TestFinder(Ctx);
  for (auto Test : TestFinder.findTests()) {
    auto ObjectFiles = AllObjectFiles();
    ExecutionResult ExecResult = Runner.runTest(Test, ObjectFiles);
    auto Result = make_unique<TestResult>(ExecResult, Test);

    for (auto Testee : TestFinder.findTestees(*Test)) {
      auto ObjectFiles = AllButOne(Testee->getParent());
      for (auto &MutationPoint : TestFinder.findMutationPoints(MutationOperators, *Testee)) {

        /// TODO: here the clone of Testee->getParent() will be used very soon instead.
        /// For now we are applying mutation to the module same as of mutation point.
        MutationPoint->applyMutation(Testee->getParent());

        auto Mutant = Compiler.CompilerModule(Testee->getParent());
        ObjectFiles.push_back(Mutant.getBinary());

        /// Rollback mutation once we have compiled the module
        MutationPoint->revertMutation();

        ExecutionResult R = Runner.runTest(Test, ObjectFiles);
        assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

        /// FIXME: Check if it's legal or not
        auto MutResult = make_unique<MutationResult>(R, std::move(MutationPoint));
        Result->addMutantResult(std::move(MutResult));
      }
    }

    Results.push_back(std::move(Result));
  }

  return Results;
}

std::vector<std::unique_ptr<TestResult>> Driver::RunGTest() {
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

//  /// FIXME: Should come from the outside
//  AddMutationOperator MutOp;
  std::vector<MutationOperator *> MutationOperators;
//  MutationOperators.push_back(&MutOp);

  GoogleTestRunner Runner;

  auto ObjectFiles = AllObjectFiles();

  auto Ctors = getStaticCtors();

  GoogleTestFinder TestFinder(Ctx);
  for (auto Test : TestFinder.findTests()) {
    ExecutionResult ExecResult = Runner.runTest(Ctors, Test, ObjectFiles);
    auto Result = make_unique<TestResult>(ExecResult, Test);

    for (auto Testee : TestFinder.findTestees(*Test)) {
      auto ObjectFiles = AllButOne(Testee->getParent());
      for (auto &MutationPoint : TestFinder.findMutationPoints(MutationOperators, *Testee)) {
        MutationPoint->applyMutation();

        auto Mutant = Compiler.CompilerModule(Testee->getParent());
        ObjectFiles.push_back(Mutant.getBinary());
        /// Rollback mutation once we have compiled the module
        MutationPoint->revertMutation();

        ExecutionResult R = Runner.runTest(Ctors, Test, ObjectFiles);
        assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

        /// FIXME: Check if it's legal or not
        auto MutResult = make_unique<MutationResult>(R, std::move(MutationPoint));
        Result->addMutantResult(std::move(MutResult));
      }
    }

    Results.push_back(std::move(Result));
  }

  fflush(stdout);
  fflush(stderr);

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

std::vector<llvm::Function *> Driver::getStaticCtors() {
  std::vector<llvm::Function *> Ctors;

  for (auto &CachedEntry : InnerCache) {
    Module *M = CachedEntry.first;

    /// Just Copied the whole logic from ExecutionEngine

    GlobalVariable *GV = M->getNamedGlobal("llvm.global_ctors");

    // If this global has internal linkage, or if it has a use, then it must be
    // an old-style (llvmgcc3) static ctor with __main linked in and in use.  If
    // this is the case, don't execute any of the global ctors, __main will do
    // it.
    if (!GV || GV->isDeclaration() || GV->hasLocalLinkage()) continue;

    // Should be an array of '{ i32, void ()* }' structs.  The first value is
    // the init priority, which we ignore.
    ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
    if (!InitList)
      continue;
    for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i) {
      ConstantStruct *CS = dyn_cast<ConstantStruct>(InitList->getOperand(i));
      if (!CS) continue;

      Constant *FP = CS->getOperand(1);
      if (FP->isNullValue())
        continue;  // Found a sentinal value, ignore.

      // Strip off constant expression casts.
      if (ConstantExpr *CE = dyn_cast<ConstantExpr>(FP))
        if (CE->isCast())
          FP = CE->getOperand(0);

      // Execute the ctor/dtor function!
      if (Function *F = dyn_cast<Function>(FP))
        Ctors.push_back(F);

      // FIXME: It is marginally lame that we just do nothing here if we see an
      // entry we don't recognize. It might not be unreasonable for the verifier
      // to not even allow this and just assert here.
    }
  }

  return Ctors;
}
