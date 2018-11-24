#include "ModuleLoader.h"
#include "TestModuleFactory.h"
#include "ForkProcessSandbox.h"

#include "Toolchain/Toolchain.h"
#include "Toolchain/Trampolines.h"
#include "Config/RawConfig.h"
#include "Toolchain/Mangler.h"

#include "CustomTestFramework/CustomTest_Test.h"
#include "CustomTestFramework/CustomTestRunner.h"

#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;
using namespace std;

using Mangler = mull::Mangler;

static TestModuleFactory SharedTestModuleFactory;
static LLVMContext context;
const static int TestTimeout = 1000;

static vector<unique_ptr<MullModule>> loadTestModules() {
  function<vector<unique_ptr<MullModule>> ()> modules = [](){
    vector<unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };
  RawConfig config;
  config.normalizeParallelizationConfig();
  FakeModuleLoader loader(context, modules);
  return loader.loadModulesFromBitcodeFileList({""}, config);
}

TEST(CustomTestRunner, noTestNameSpecified) {
  RawConfig config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.mangler());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {}, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  Trampolines trampolines({});
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, jit);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, tooManyParameters) {
  RawConfig config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.mangler());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "arg1", "arg2" }, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  Trampolines trampolines({});
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, jit);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, runPassingTest) {
  RawConfig config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.mangler());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "passing_test" }, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  Trampolines trampolines({});
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, jit);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Passed);
}

TEST(CustomTestRunner, runFailingTest) {
  RawConfig config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.mangler());

  Function *constructor = nullptr;
  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    Module *module = m->getModule();
    if (!constructor) {
      constructor = module->getFunction("initGlobalVariable");
    }
    auto object = toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "failing_test" }, nullptr, { constructor });
  ForkProcessSandbox sandbox;
  JITEngine jit;
  Trampolines trampolines({});
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, jit);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, attemptToRunUnknownTest) {
  RawConfig config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.mangler());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "foobar" }, nullptr, {});
  ForkProcessSandbox sandbox;
  JITEngine jit;
  Trampolines trampolines({});
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, jit);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}
