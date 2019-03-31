#include "mull/Config/Configuration.h"
#include "mull/ForkProcessSandbox.h"
#include "mull/ModuleLoader.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTestRunner.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTest_Test.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Toolchain.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/IR/Module.h>

#include "FixturePaths.h"
#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;
using namespace std;

using Mangler = mull::Mangler;

const static int TestTimeout = 1000;

TEST(CustomTestRunner, noTestNameSpecified) {
  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};
  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  for (auto &m : loadedModules) {
    auto object =
        toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {}, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result =
      sandbox.run([&]() { return runner.runTest(&test, jit); }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, tooManyParameters) {
  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};
  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;

  for (auto &m : loadedModules) {
    auto object =
        toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {"arg1", "arg2"}, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result =
      sandbox.run([&]() { return runner.runTest(&test, jit); }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, runPassingTest) {
  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};
  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;

  for (auto &m : loadedModules) {
    auto object =
        toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {"passing_test"}, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result =
      sandbox.run([&]() { return runner.runTest(&test, jit); }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Passed);
}

TEST(CustomTestRunner, runFailingTest) {
  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};
  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);

  Function *constructor = nullptr;
  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  for (auto &m : loadedModules) {
    Module *module = m->getModule();
    if (!constructor) {
      constructor = module->getFunction("initGlobalVariable");
    }
    auto object =
        toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {"failing_test"}, nullptr,
                       {constructor});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result =
      sandbox.run([&]() { return runner.runTest(&test, jit); }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, attemptToRunUnknownTest) {
  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};
  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  for (auto &m : loadedModules) {
    auto object =
        toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {"foobar"}, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result =
      sandbox.run([&]() { return runner.runTest(&test, jit); }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}
