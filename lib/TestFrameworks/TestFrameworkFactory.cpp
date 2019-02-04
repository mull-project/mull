
#include <TestFrameworks/TestFrameworkFactory.h>

#include "TestFrameworks/TestFrameworkFactory.h"

#include "TestFrameworks/CustomTestFramework/CustomTestFramework.h"
#include "TestFrameworks/GoogleTest/GoogleTest.h"
#include "TestFrameworks/SimpleTest/SimpleTest.h"

#include "Config/Configuration.h"
#include "Toolchain/Toolchain.h"

using namespace mull;
using namespace llvm;

TestFramework
TestFrameworkFactory::createTestFramework(const std::string &name,
                                          Toolchain &toolchain,
                                          Configuration &configuration) {
  if (name == "GoogleTest") {
    return googleTestFramework(toolchain, configuration);
  }

  if (name == "SimpleTest") {
    return simpleTestFramework(toolchain, configuration);
  }

  if (name == "CustomTest") {
    return customTestFramework(toolchain, configuration);
  }

  std::string errorMessage("Unknown test framework: ");
  errorMessage += name;
  llvm_unreachable(errorMessage.c_str());
}

TestFramework
TestFrameworkFactory::simpleTestFramework(Toolchain &toolchain,
                                          Configuration &configuration) {
  auto finder = make_unique<SimpleTestFinder>();
  auto runner = make_unique<SimpleTestRunner>(toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework
TestFrameworkFactory::googleTestFramework(Toolchain &toolchain,
                                          Configuration &configuration) {
  auto finder = make_unique<GoogleTestFinder>();
  auto runner = make_unique<GoogleTestRunner>(toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework
TestFrameworkFactory::customTestFramework(Toolchain &toolchain,
                                          Configuration &configuration) {
  auto finder = make_unique<CustomTestFinder>(configuration.customTests);
  auto runner = make_unique<CustomTestRunner>(toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

std::vector<std::pair<std::string, std::string>>
TestFrameworkFactory::commandLineOptions() {
  std::vector<std::pair<std::string, std::string>> options(
      {std::make_pair("GoogleTest", "Google Test Framework"),
       std::make_pair("CustomTest", "Custom Test Framework"),
       std::make_pair("SimpleTest", "Simple Test (For internal usage only)")});

  return options;
}
