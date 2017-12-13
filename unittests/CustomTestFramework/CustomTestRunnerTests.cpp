#include "ModuleLoader.h"
#include "TestModuleFactory.h"
#include "ForkProcessSandbox.h"

#include "Toolchain/Toolchain.h"
#include "Config.h"

#include "CustomTestFramework/CustomTest_Test.h"
#include "CustomTestFramework/CustomTestRunner.h"

#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;
using namespace std;

static TestModuleFactory SharedTestModuleFactory;
static LLVMContext context;
const static int TestTimeout = 1000;

static vector<MullModule> loadTestModules() {
  function<vector<MullModule> ()> modules = [](){
    vector<MullModule> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };

  FakeModuleLoader loader(context, modules);
  return loader.loadModulesFromBitcodeFileList({""});
}

TEST(CustomTestRunner, noTestNameSpecified) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(m);
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {}, nullptr, {});
  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, objects);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, tooManyParameters) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(m);
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "arg1", "arg2" }, nullptr, {});
  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, objects);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, runPassingTest) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(m);
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "passing_test" }, nullptr, {});
  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, objects);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Passed);
}

TEST(CustomTestRunner, runFailingTest) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  Function *constructor = nullptr;
  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    Module *module = m.getModule();
    if (!constructor) {
      constructor = module->getFunction("initGlobalVariable");
    }
    auto object = toolchain.compiler().compileModule(m);
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "failing_test" }, nullptr, { constructor });
  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, objects);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}

TEST(CustomTestRunner, attemptToRunUnknownTest) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(m);
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "foobar" }, nullptr, {});
  ForkProcessSandbox sandbox;
  ExecutionResult result = sandbox.run([&]() {
    return runner.runTest(&test, objects);
  }, TestTimeout);
  ASSERT_EQ(result.status, ExecutionStatus::Failed);
}
