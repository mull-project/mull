#include "Driver.h"
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
#include <chrono>
#include <fstream>

using namespace llvm;
using namespace llvm::object;
using namespace Mutang;
using namespace std;
using namespace std::chrono;

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
  std::vector<std::unique_ptr<TestResult>> Results;

  /// Assumption: all modules will be used during the execution
  /// Therefore we load them into memory and compile immediately
  /// Later on modules used only for generating of mutants
  for (auto ModulePath : Cfg.getBitcodePaths()) {
    unique_ptr<MutangModule> ownedModule = Loader.loadModuleAtPath(ModulePath);
    MutangModule &module = *ownedModule.get();
    assert(ownedModule && "Can't load module");

    ObjectFile *objectFile = toolchain.cache().getObject(module);

    if (objectFile == nullptr) {
      auto owningObjectFile = toolchain.compiler().compileModule(*module.clone().get());
      objectFile = owningObjectFile.getBinary();
      toolchain.cache().putObject(std::move(owningObjectFile), *ownedModule.get());
    }

    InnerCache.insert(std::make_pair(module.getModule(), objectFile));
    Ctx.addModule(std::move(ownedModule));
  }

  auto foundTests = Finder.findTests(Ctx);

  //outs() << "Driver::Run::begin with " << foundTests.size() << " tests\n";

  for (auto &test : foundTests) {
    auto ObjectFiles = AllObjectFiles();

    //outs() << "\tDriver::Run::run test: " << test->getTestName() << "\n";

    ExecutionResult ExecResult = Sandbox->run([&](ExecutionResult *SharedResult) {
      *SharedResult = Runner.runTest(test.get(), ObjectFiles);
    }, Cfg.getTimeout());

    auto BorrowedTest = test.get();
    auto Result = make_unique<TestResult>(ExecResult, std::move(test));

    auto testees = Finder.findTestees(BorrowedTest, Ctx, Cfg.getMaxDistance());

    //outs() << "\tagainst " << testees.size() << " testees\n";

    for (auto testee : testees) {
      auto MPoints = Finder.findMutationPoints(Ctx, *(testee.first));
      if (MPoints.size() == 0) {
        continue;
      }

      //outs() << "\t\tDriver::Run::process testee: " << testee.first->getName() << "\n";
      //outs() << "\t\tagainst " << MPoints.size() << " mutation points\n";

      auto ObjectFiles = AllButOne(testee.first->getParent());
      for (auto mutationPoint : MPoints) {
//        outs() << "\t\t\tDriver::Run::run mutant:" << "\t";
//        mutationPoint->getOriginalValue()->print(outs());
//        outs() << "\n";

        ExecutionResult result;
        bool dryRun = Cfg.isDryRun();
        if (dryRun) {
          result.Status = DryRun;
          result.RunningTime = ExecResult.RunningTime * 10;
        } else {
          ObjectFile *mutant = toolchain.cache().getObject(*mutationPoint);
          if (mutant == nullptr) {
            auto owningObject = mutationPoint->applyMutation(toolchain.compiler());
            mutant = owningObject.getBinary();
            toolchain.cache().putObject(std::move(owningObject), *mutationPoint);
          }
          ObjectFiles.push_back(mutant);

          result = Sandbox->run([&](ExecutionResult *SharedResult) {
            ExecutionResult R = Runner.runTest(BorrowedTest, ObjectFiles);

            assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

            *SharedResult = R;
          }, ExecResult.RunningTime * 10);
          ObjectFiles.pop_back();

          assert(result.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
        }

        auto MutResult = make_unique<MutationResult>(result, mutationPoint, testee.second);
        Result->addMutantResult(std::move(MutResult));
      }
    }

    Results.push_back(std::move(Result));
  }

//  outs() << "Driver::Run::end\n";

  return Results;
}

std::vector<llvm::object::ObjectFile *> Driver::AllButOne(llvm::Module *One) {
  std::vector<llvm::object::ObjectFile *> Objects;

  for (auto &CachedEntry : InnerCache) {
    if (One != CachedEntry.first) {
      Objects.push_back(CachedEntry.second);
    }
  }

  return Objects;
}

std::vector<llvm::object::ObjectFile *> Driver::AllObjectFiles() {
  std::vector<llvm::object::ObjectFile *> Objects;

  for (auto &CachedEntry : InnerCache) {
    Objects.push_back(CachedEntry.second);
  }

  return Objects;
}

#pragma mark - Debug

void Driver::debug_PrintTestNames() {
  for (auto ModulePath : Cfg.getBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
//    assert(OwnedModule && "Can't load module");
//    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    outs() << Test->getTestName() << "\n";
  }
}

void Driver::debug_PrintTesteeNames() {
  for (auto ModulePath : Cfg.getBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
//    assert(OwnedModule && "Can't load module");
//    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    outs() << Test->getTestName() << "\n";
    for (auto &testee : Finder.findTestees(Test.get(), Ctx, Cfg.getMaxDistance())) {
      outs() << "\t" << testee.first->getName() << "\n";
    }
  }
}

void Driver::debug_PrintMutationPoints() {
  for (auto ModulePath : Cfg.getBitcodePaths()) {
    auto OwnedModule = Loader.loadModuleAtPath(ModulePath);
//    assert(OwnedModule && "Can't load module");
//    Ctx.addModule(std::move(OwnedModule));
  }

  for (auto &Test : Finder.findTests(Ctx)) {
    outs() << Test->getTestName() << "\n";
    for (auto testee : Finder.findTestees(Test.get(), Ctx, Cfg.getMaxDistance())) {
      auto MPoints = Finder.findMutationPoints(Ctx, *(testee.first));
      if (MPoints.size()) {
        outs() << "\t" << testee.first->getName() << "\n";
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
