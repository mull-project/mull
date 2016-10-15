#include "Driver.h"
#include "Compiler.h"
#include "Config.h"
#include "Context.h"
#include "ModuleLoader.h"
#include "TestResult.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "TestFinder.h"
#include "TestRunner.h"

/// FIXME: Should be abstract
#include "MutationOperators/AddMutationOperator.h"

#include <algorithm>
#include <fstream>

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

  auto foundTests = Finder.findTests(Ctx);

  outs() << "Driver::Run::begin with " << foundTests.size() << " tests\n";

  for (auto &test : foundTests) {
    auto ObjectFiles = AllObjectFiles();

    outs() << "\tDriver::Run::run test: " << test->getTestName() << "\n";

    ExecutionResult ExecResult = Sandbox->run([&](ExecutionResult *SharedResult){
      *SharedResult = Runner.runTest(test.get(), ObjectFiles);
    });

    auto BorrowedTest = test.get();
    auto Result = make_unique<TestResult>(ExecResult, std::move(test));

    auto testees = Finder.findTestees(BorrowedTest, Ctx);

    outs() << "\tagainst " << testees.size() << " testees\n";

    for (auto Testee : testees) {
      auto MPoints = Finder.findMutationPoints(*Testee);
      if (MPoints.size() == 0) {
        continue;
      }

      outs() << "\t\tDriver::Run::process testee: " << Testee->getName() << "\n";
      outs() << "\t\tagainst " << MPoints.size() << " mutation points\n";

      for (auto mutationPoint : MPoints) {
        auto ObjectFiles = AllButOne(Testee->getParent());
        outs() << "\t\t\tDriver::Run::run mutant:" << "\t";
        mutationPoint->getOriginalValue()->print(outs());
        outs() << "\n";

        auto OwnedCopy = CloneModule(Testee->getParent());
        auto BorrowedCopy = OwnedCopy.get();

        verifyModule(*BorrowedCopy, &errs());
        ExecutionResult R = Sandbox->run([&](ExecutionResult *SharedResult) {
          mutationPoint->applyMutation(BorrowedCopy);

          auto Mutant = Compiler.CompilerModule(BorrowedCopy);
          ObjectFiles.push_back(Mutant.getBinary());

          ExecutionResult R = Runner.runTest(BorrowedTest, ObjectFiles);

          assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

          *SharedResult = R;
        });

        /// It's happening now when a child crashes
        // assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

        auto MutResult = make_unique<MutationResult>(R, mutationPoint);
        Result->addMutantResult(std::move(MutResult));
      }
    }

    Results.push_back(std::move(Result));
  }

  outs() << "Driver::Run::end\n";

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
    outs() << Test->getTestName() << "\n";
  }
}

void Driver::debug_PrintTesteeNames() {
  for (auto ModulePath : Cfg.GetBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
    assert(OwnedModule && "Can't load module");
    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    outs() << Test->getTestName() << "\n";
    for (auto &Testee : Finder.findTestees(Test.get(), Ctx)) {
      outs() << "\t" << Testee->getName() << "\n";
    }
  }
}

void Driver::debug_PrintMutationPoints() {
  for (auto ModulePath : Cfg.GetBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
    assert(OwnedModule && "Can't load module");
    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    outs() << Test->getTestName() << "\n";
    for (auto &Testee : Finder.findTestees(Test.get(), Ctx)) {
      auto MPoints = Finder.findMutationPoints(*Testee);
      if (MPoints.size()) {
        outs() << "\t" << Testee->getName() << "\n";
      }
      for (auto &MPoint : MPoints) {
        outs() << "\t\t";
        MPoint->getOriginalValue()->print(outs());
        outs() << "\n";

        if (Instruction *I = dyn_cast<Instruction>(MPoint->getOriginalValue())) {
          auto DL = I->getDebugLoc();
          auto Filename = DL->getFilename();
          auto LineNo = DL->getLine();

          std::string line;
          std::ifstream SourceFile(Filename);
          unsigned int curLine = 1;
          if (SourceFile.is_open()) {
            while (!SourceFile.eof()) {
              getline(SourceFile, line);
              if (curLine == LineNo) {
                outs() << "\t\t";
                outs() << Filename << ":" << LineNo;
                outs() << "\n";

                outs() << "\t\t";
                outs() << line;
                outs() << "\n";
                break;
              }
              curLine++;
            }
            SourceFile.close();
          } else {
            errs() << "Unable to open file";
          }
        }
      }
//      outs() << "\n";
    }
//    outs() << "\n";
  }
}
