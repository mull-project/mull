#include "Driver.h"
#include "Config.h"
#include "Context.h"
#include "ModuleLoader.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

/// FIXME: Should be abstract
#include "TestFinders/SimpleTestFinder.h"

/// FIXME: Should be abstract
#include "MutationOperators/AddMutationOperator.h"

//using namespace llvm;
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

void Driver::Run() {
  Context Ctx;
  for (auto ModulePath : Cfg.GetBitcodePaths()) {
    auto Module = Loader.loadModuleAtPath(ModulePath);
    assert(Module && "Can't load module");
    Ctx.addModule(std::move(Module));
  }

  SimpleTestFinder TestFinder(Ctx);
  for (auto Test : TestFinder.findTests()) {
    for (auto Testee : TestFinder.findTestees(*Test)) {
      /// FIXME: Should come from the outside
      AddMutationOperator MutOp;
      std::vector<MutationOperator *> MutationOperators;
      MutationOperators.push_back(&MutOp);
      for (auto &Testee : TestFinder.findMutationPoints(MutationOperators, *Testee)) {
        Testee->getOriginalValue()->dump();
      }
    }
  }
}
