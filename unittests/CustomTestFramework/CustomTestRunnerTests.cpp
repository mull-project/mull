#include "ModuleLoader.h"
#include "TestModuleFactory.h"

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

static vector<unique_ptr<MullModule>> loadTestModules() {
  function<vector<unique_ptr<MullModule>> ()> modules = [](){
    vector<unique_ptr<MullModule>> modules;
    {
      auto module = SharedTestModuleFactory.createCustomTest_Distance_DistanceModule();
      modules.push_back(make_unique<MullModule>(move(module),
                                                "CustomTest_Distance_DistanceModule"));
    }
    {
      auto module = SharedTestModuleFactory.createCustomTest_Distance_MainModule();
      modules.push_back(make_unique<MullModule>(move(module),
                                                "CustomTest_Distance_MainModule"));
    }
    {
      auto module = SharedTestModuleFactory.createCustomTest_Distance_TestModule();
      modules.push_back(make_unique<MullModule>(move(module),
                                                "CustomTest_Distance_TestModule"));
    }

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
    auto object = toolchain.compiler().compileModule(*m.get());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", {}, nullptr, {});
  ExecutionResult result = runner.runTest(&test, objects);
  ASSERT_EQ(result.Status, ExecutionStatus::Failed);
  ASSERT_EQ(result.exitStatus, 48);
}

TEST(CustomTestRunner, tooManyParameters) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m.get());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "arg1", "arg2" }, nullptr, {});
  ExecutionResult result = runner.runTest(&test, objects);
  ASSERT_EQ(result.Status, ExecutionStatus::Failed);
  ASSERT_EQ(result.exitStatus, 93);
}

TEST(CustomTestRunner, runPassingTest) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m.get());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "passing_test" }, nullptr, {});
  ExecutionResult result = runner.runTest(&test, objects);
  ASSERT_EQ(result.Status, ExecutionStatus::Passed);
  ASSERT_EQ(result.exitStatus, 0);
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
    Module *module = m.get()->getModule();
    if (!constructor) {
      constructor = module->getFunction("initGlobalVariable");
    }
    auto object = toolchain.compiler().compileModule(*m.get());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "failing_test" }, nullptr, { constructor });
  ExecutionResult result = runner.runTest(&test, objects);
  ASSERT_EQ(result.Status, ExecutionStatus::Failed);
  ASSERT_EQ(result.exitStatus, 112);
}

TEST(CustomTestRunner, attemptToRunUnknownTest) {
  Config config;
  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  auto loadedModules = loadTestModules();
  for (auto &m : loadedModules) {
    auto object = toolchain.compiler().compileModule(*m.get());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  CustomTest_Test test("test", "mull", { "foobar" }, nullptr, {});
  ExecutionResult result = runner.runTest(&test, objects);
  ASSERT_EQ(result.Status, ExecutionStatus::Failed);
  ASSERT_EQ(result.exitStatus, 120);
}