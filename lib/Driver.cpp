#include "Driver.h"

#include "Config.h"
#include "Context.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "Result.h"
#include "TestResult.h"

#include "TestFinder.h"
#include "TestRunner.h"

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

uint64_t *_callTreeMapping = nullptr;
std::stack<uint64_t> _callstack;

extern "C" void mull_enterFunction(uint64_t functionIndex) {
  assert(_callTreeMapping);
  DynamicCallTree::enterFunction(functionIndex, _callTreeMapping, _callstack);
}

extern "C" void mull_leaveFunction(uint64_t functionIndex) {
  assert(_callTreeMapping);
  DynamicCallTree::leaveFunction(functionIndex, _callTreeMapping, _callstack);
}

}

Driver::~Driver() {
  delete this->Sandbox;
  delete this->diagnostics;
  munmap(_callTreeMapping, functions.size());
  _callTreeMapping = nullptr;
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
      toolchain.cache().putObject(std::move(owningObjectFile), *ownedModule.get());
    }

    InnerCache.insert(std::make_pair(module.getModule(), objectFile));
    Ctx.addModule(std::move(ownedModule));
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

    ExecutionResult ExecResult = Sandbox->run([&](ExecutionResult *SharedResult) {
      *SharedResult = Runner.runTest(test.get(), ObjectFiles);
    }, Cfg.getTimeout());

    if (ExecResult.Status != Passed) {
      Logger::error() << "error: Test has failed: " << test->getTestName() << "\n";
      continue;
    }

    auto BorrowedTest = test.get();
    auto Result = make_unique<TestResult>(ExecResult, std::move(test));

    auto testees = Finder.findTestees(BorrowedTest, Ctx, Cfg.getMaxDistance());

    /// -1 since we are skipping the first testee
    const int testeesCount = testees.size() - 1;

    Logger::debug().indent(4)
      << "Driver::Run> found "
      << testeesCount << " testees\n";

    int testeeIndex = 1;
    for (auto testee_it = std::next(testees.begin()), ee = testees.end();
         testee_it != ee;
         ++testee_it) {

      auto &&testee = *testee_it;

      Logger::debug().indent(8)
        << "Driver::Run::process testee "
        << "[" << testeeIndex++ << "/" << testeesCount << "]: "
        << testee->getTesteeFunction()->getName()
        << ", ";

      auto MPoints = Finder.findMutationPoints(Ctx, *(testee->getTesteeFunction()));
      if (MPoints.empty()) {
        Logger::debug() << "no mutation points, skipping.\n";

        continue;
      }

      Logger::debug() << "against " << MPoints.size() << " mutation points\n";
      Logger::debug().indent(8) << "";

      auto ObjectFiles = AllButOne(testee->getTesteeFunction()->getParent());
      for (auto mutationPoint : MPoints) {
        //        Logger::info() << "\t\t\tDriver::Run::run mutant:" << "\t";
        //        mutationPoint->getOriginalValue()->print(Logger::info());
        //        Logger::info() << "\n";

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

  //Logger::info() << "Driver::Run::end\n";

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
  auto parameterType = Type::getInt64Ty(context);
  auto voidType = Type::getVoidTy(context);
  std::vector<Type *> parameterTypes(1, parameterType);

  FunctionType *callbackType = FunctionType::get(voidType, parameterTypes, false);

  Value *functionIndex = ConstantInt::get(parameterType, index);
  std::vector<Value *> parameters(1, functionIndex);

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

std::vector<std::unique_ptr<MutationOperator>>
Driver::mutationOperators(std::vector<std::string> mutationOperatorStrings) {
    if (mutationOperatorStrings.size() == 0) {
      Logger::info()
        << "Driver> No mutation operators specified in a config file. "
        << "Defaulting to default operators:" << "\n";

      auto mutationOperators = defaultMutationOperators();

      for (auto &mutationOperator: mutationOperators) {
        Logger::info() << "\t" << mutationOperator.get()->uniqueID() << "\n";
      }

      return mutationOperators;
    }

    std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
    for (auto mutation : mutationOperatorStrings) {
      if (mutation == AddMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<AddMutationOperator>());
      }
      else if (mutation == AndOrReplacementMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());
      }
      else if (mutation == MathSubMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<MathSubMutationOperator>());
      }
      else if (mutation == MathMulMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<MathMulMutationOperator>());
      }
      else if (mutation == MathDivMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<MathDivMutationOperator>());
      }
      else if (mutation == NegateConditionMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());
      }
      else if (mutation == RemoveVoidFunctionMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());
      }
      else if (mutation == ScalarValueMutationOperator::ID) {
        mutationOperators.emplace_back(make_unique<ScalarValueMutationOperator>());
      }
      else {
        Logger::info() << "Driver> Unknown Mutation Operator: " << mutation << "\n";
      }
    }

    if (mutationOperators.size() == 0) {
      Logger::info()
        << "Driver> No valid mutation operators found in a config file.\n";
    }

    return mutationOperators;
}

std::vector<std::unique_ptr<MutationOperator>>
Driver::defaultMutationOperators() {
  std::vector<std::unique_ptr<MutationOperator>> operators;

  operators.emplace_back(make_unique<AddMutationOperator>());
  operators.emplace_back(make_unique<NegateConditionMutationOperator>());
  operators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());

  return operators;
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
