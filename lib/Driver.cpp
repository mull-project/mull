#include "Driver.h"

#include "Config.h"
#include "Context.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "Result.h"
#include "TestResult.h"
#include "TestFinder.h"
#include "TestRunner.h"
#include "MutationsFinder.h"

#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <vector>
#include <sys/mman.h>
#include <sys/types.h>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;
using namespace std::chrono;

namespace mull {

extern "C" void mull_enterFunction(Driver *driver, uint64_t functionIndex) {
  assert(driver);
  assert(driver->callTreeMapping());
  DynamicCallTree::enterFunction(functionIndex,
                                 driver->callTreeMapping(),
                                 driver->callstack());
}

extern "C" void mull_leaveFunction(Driver *driver, uint64_t functionIndex) {
  assert(driver);
  assert(driver->callTreeMapping());
  DynamicCallTree::leaveFunction(functionIndex,
                                 driver->callTreeMapping(),
                                 driver->callstack());
}

}

Driver::~Driver() {
  delete this->Sandbox;
  delete this->diagnostics;
  munmap(_callTreeMapping, functions.size());
}

/// Populate mull::Context with modules using
/// ModulePaths from mull::Config.
/// mull::Context should be populated using ModuleLoader
/// so that we could inject modules from string for testing purposes

/// Having mull::Context in place we could instantiate TestFinder and find all tests
/// Using same TestFinder we could find mutation points, apply them sequentially and
/// run tests/mutants using newly created TestRunner

/// This method should return (somehow) results of the tests/mutants execution
/// So that we could easily plug in some TestReporter

/// UPD: The method returns set of results
/// Number of results equals to a number of tests
/// Each result contains result of execution of an original test and
/// all the results of each mutant within corresponding MutationPoint

std::unique_ptr<Result> Driver::Run() {
  std::vector<std::unique_ptr<TestResult>> Results;
  std::vector<std::unique_ptr<Testee>> allTestees;

  /// Assumption: all modules will be used during the execution
  /// Therefore we load them into memory and compile immediately
  /// Later on modules used only for generating of mutants
  std::vector<std::string> bitcodePaths = Cfg.getBitcodePaths();
  std::vector<unique_ptr<MullModule>> modules =
    Loader.loadModulesFromBitcodeFileList(bitcodePaths);

  for (auto &ownedModule : modules) {
    MullModule &module = *ownedModule.get();
    assert(ownedModule && "Can't load module");
    Ctx.addModule(std::move(ownedModule));

    for (auto &function: module.getModule()->getFunctionList()) {
      if (function.isDeclaration()) {
        continue;
      }
      CallTreeFunction callTreeFunction(&function);
      uint64_t index = functions.size();
      functions.push_back(callTreeFunction);
      injectCallbacks(&function, index);
    }

    ObjectFile *objectFile = toolchain.cache().getObject(module);

    if (objectFile == nullptr) {
      auto owningObjectFile = toolchain.compiler().compileModule(*module.clone().get());
      objectFile = owningObjectFile.getBinary();
      toolchain.cache().putObject(std::move(owningObjectFile), module);
    }

    InnerCache.insert(std::make_pair(module.getModule(), objectFile));
  }

  prepareForExecution();

  auto foundTests = Finder.findTests(Ctx);
  const int testsCount = foundTests.size();

  Logger::debug() << "Driver::Run> found "
                  << testsCount
                  << " tests\n";

  int testIndex = 1;
  for (auto &test : foundTests) {
    auto ObjectFiles = AllObjectFiles();

    Logger::debug().indent(4)
      << "Driver::Run> current test "
      << "[" << testIndex++ << "/" << testsCount << "]: "
      << test->getTestName()
      << "\n";

    _callstack = stack<uint64_t>();
    ExecutionResult ExecResult = Sandbox->run([&](ExecutionResult *SharedResult) {
      *SharedResult = Runner.runTest(test.get(), ObjectFiles);
    }, Cfg.getTimeout());

    if (ExecResult.Status != Passed) {
      Logger::error() << "error: Test has failed: " << test->getTestName() << "\n";
      continue;
    }

    auto BorrowedTest = test.get();
    auto Result = make_unique<TestResult>(ExecResult, std::move(test));

    std::unique_ptr<CallTree> callTree(dynamicCallTree.createCallTree());

    auto subtrees = dynamicCallTree.extractTestSubtrees(callTree.get(), BorrowedTest);
    auto testees = dynamicCallTree.createTestees(subtrees, BorrowedTest,
                                                 Cfg.getMaxDistance(), filter);

    dynamicCallTree.cleanupCallTree(std::move(callTree));
    if (testees.empty()) {
      Logger::error() << "error: Coult not find any testees: " << BorrowedTest->getTestName() << "\n";
      continue;
    }

    /// -1 since we are skipping the first testee
    const int testeesCount = testees.size() - 1;

    Logger::debug().indent(4)
      << "Driver::Run> found "
      << testeesCount << " testees\n";

    int testeeIndex = 1;
    for (auto testee_it = std::next(testees.begin()), ee = testees.end();
         testee_it != ee;
         ++testee_it) {

      std::unique_ptr<Testee> &testee = *testee_it;

      Logger::debug().indent(8)
        << "Driver::Run::process testee "
        << "[" << testeeIndex++ << "/" << testeesCount << "]: "
        << testee->getTesteeFunction()->getName()
        << ", ";

      auto MPoints = mutationsFinder.getMutationPoints(Ctx, *testee.get(), filter);
      if (MPoints.empty()) {
        Logger::debug() << "no mutation points, skipping.\n";

        continue;
      }

      Logger::debug() << "against " << MPoints.size() << " mutation points\n";
      Logger::debug().indent(8) << "";

      auto ObjectFiles = AllButOne(testee->getTesteeFunction()->getParent());
      for (auto mutationPoint : MPoints) {

        Logger::debug() << ".";

        ExecutionResult result;
        bool dryRun = Cfg.isDryRun();
        if (dryRun) {
          result.Status = DryRun;
          result.RunningTime = ExecResult.RunningTime * 10;
        } else {
          ObjectFile *mutant = toolchain.cache().getObject(*mutationPoint);
          if (mutant == nullptr) {
            auto ownedModule = mutationPoint->cloneModuleAndApplyMutation();

            auto owningObject = toolchain.compiler().compileModule(ownedModule.get());

            mutant = owningObject.getBinary();
            toolchain.cache().putObject(std::move(owningObject), *mutationPoint);
          }
          ObjectFiles.push_back(mutant);

          const auto sandboxTimeout = std::max(30LL,
                                               ExecResult.RunningTime * 10);

          result = Sandbox->run([&](ExecutionResult *SharedResult) {
            ExecutionResult R = Runner.runTest(BorrowedTest, ObjectFiles);

            assert(R.Status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

            *SharedResult = R;
          }, sandboxTimeout);
          ObjectFiles.pop_back();

          assert(result.Status != ExecutionStatus::Invalid &&
                 "Expect to see valid TestResult");
        }

        diagnostics->report(mutationPoint, result.Status);

        auto MutResult = make_unique<MutationResult>(result, mutationPoint, testee.get());
        Result->addMutantResult(std::move(MutResult));
      }

      Logger::debug() << "\n";
    }

    allTestees.insert(allTestees.end(),std::make_move_iterator(testees.begin()),
                      std::make_move_iterator(testees.end()));

    Results.push_back(std::move(Result));
  }

  std::unique_ptr<Result> result = make_unique<Result>(std::move(Results),
                                                       std::move(allTestees));

  return result;
}

void Driver::prepareForExecution() {
  assert(_callTreeMapping == nullptr && "Called twice?");
  assert(functions.size() > 1 && "Functions must be filled in before this call");

  /// Creating a memory to be shared between child and parent.
  _callTreeMapping = (uint64_t *) mmap(NULL,
                                       sizeof(uint64_t) * functions.size(),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_ANONYMOUS,
                                       -1,
                                       0);
  memset(_callTreeMapping, 0, functions.size());
  dynamicCallTree.prepare(_callTreeMapping);
}

void Driver::injectCallbacks(llvm::Function *function, uint64_t index) {
  auto &context = function->getParent()->getContext();
  auto int64Type = Type::getInt64Ty(context);
  auto driverPointerType = Type::getVoidTy(context)->getPointerTo();
  auto voidType = Type::getVoidTy(context);
  std::vector<Type *> parameterTypes({driverPointerType, int64Type});

  FunctionType *callbackType = FunctionType::get(voidType, parameterTypes, false);

  Value *functionIndex = ConstantInt::get(int64Type, index);
  uint32_t pointerWidth = toolchain.targetMachine().createDataLayout().getPointerSize();
  ConstantInt *driverPointerAddress = ConstantInt::get(context, APInt(pointerWidth * 8, (orc::TargetAddress)this));
  Value *driverPointer = ConstantExpr::getCast(Instruction::IntToPtr,
                                               driverPointerAddress,
                                               int64Type->getPointerTo());
  std::vector<Value *> parameters({driverPointer, functionIndex});

  Function *enterFunction = function->getParent()->getFunction("mull_enterFunction");
  Function *leaveFunction = function->getParent()->getFunction("mull_leaveFunction");

  if (enterFunction == nullptr && leaveFunction == nullptr) {
    enterFunction = Function::Create(callbackType,
                                     Function::ExternalLinkage,
                                     "mull_enterFunction",
                                     function->getParent());

    leaveFunction = Function::Create(callbackType,
                                     Function::ExternalLinkage,
                                     "mull_leaveFunction",
                                     function->getParent());
  }

  assert(enterFunction);
  assert(leaveFunction);

  auto &entryBlock = *function->getBasicBlockList().begin();
  CallInst *enterFunctionCall = CallInst::Create(enterFunction, parameters);
  enterFunctionCall->insertBefore(&*entryBlock.getInstList().begin());

  for (auto &block : function->getBasicBlockList()) {
    ReturnInst *returnStatement = nullptr;
    if (!(returnStatement = dyn_cast<ReturnInst>(block.getTerminator()))) {
      continue;
    }

    CallInst *leaveFunctionCall = CallInst::Create(leaveFunction, parameters);
    leaveFunctionCall->insertBefore(returnStatement);
  }
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
