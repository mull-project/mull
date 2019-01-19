#include "TestFrameworks/TestFrameworkFactory.h"

#include "TestFrameworks/CustomTestFramework/CustomTestFramework.h"
#include "TestFrameworks/GoogleTest/GoogleTest.h"
#include "TestFrameworks/SimpleTest/SimpleTest.h"

#include "Config/Configuration.h"
#include "Toolchain/Toolchain.h"

using namespace mull;
using namespace llvm;

TestFrameworkFactory::TestFrameworkFactory(Configuration &configuration,
                                           Toolchain &toolchain)
    : configuration(configuration), toolchain(toolchain) {}

TestFramework
TestFrameworkFactory::createTestFramework(const std::string &name) {
  if (name == "GoogleTest") {
    return googleTestFramework();
  }

  if (name == "SimpleTest") {
    return simpleTestFramework();
  }

  if (name == "CustomTest") {
    return customTestFramework();
  }

  std::string errorMessage("Unknown test framework: ");
  errorMessage += name;
  llvm_unreachable(errorMessage.c_str());
}

TestFramework TestFrameworkFactory::simpleTestFramework() {
  auto finder = make_unique<SimpleTestFinder>();
  auto runner = make_unique<SimpleTestRunner>(toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework TestFrameworkFactory::googleTestFramework() {
  auto finder = make_unique<GoogleTestFinder>();
  auto runner = make_unique<GoogleTestRunner>(toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework TestFrameworkFactory::customTestFramework() {
  auto finder = make_unique<CustomTestFinder>(configuration.customTests);
  auto runner = make_unique<CustomTestRunner>(toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}
