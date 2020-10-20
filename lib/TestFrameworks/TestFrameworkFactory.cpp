#include "mull/TestFrameworks/TestFrameworkFactory.h"

#include "mull/Config/Configuration.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTestFinder.h"
#include "mull/TestFrameworks/BoostTest/BoostTestFinder.h"
#include "mull/TestFrameworks/CppUTest/CppUTestFinder.h"
#include "mull/TestFrameworks/GoogleTest/GoogleTestFinder.h"
#include "mull/TestFrameworks/NativeTestRunner.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "mull/Toolchain/Toolchain.h"

using namespace mull;
using namespace llvm;

TestFramework TestFrameworkFactory::createTestFramework(const std::string &name,
                                                        Toolchain &toolchain,
                                                        Configuration &configuration,
                                                        Diagnostics &diagnostics) {
  if (name == "BoostTest") {
    return boostTestFramework(toolchain, configuration, diagnostics);
  }

  if (name == "GoogleTest") {
    return googleTestFramework(toolchain, configuration, diagnostics);
  }

  if (name == "CppUTest") {
    return cppuTestFramework(toolchain, configuration, diagnostics);
  }

  if (name == "SimpleTest") {
    return simpleTestFramework(toolchain, configuration, diagnostics);
  }

  if (name == "CustomTest") {
    return customTestFramework(toolchain, configuration, diagnostics);
  }

  std::string errorMessage("Unknown test framework: ");
  errorMessage += name;
  llvm_unreachable(errorMessage.c_str());
}

TestFramework TestFrameworkFactory::boostTestFramework(Toolchain &toolchain,
                                                        Configuration &configuration,
                                                        Diagnostics &diagnostics) {
  auto finder = std::make_unique<BoostTestFinder>(diagnostics);
  auto runner = std::make_unique<NativeTestRunner>(diagnostics, toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework TestFrameworkFactory::simpleTestFramework(Toolchain &toolchain,
                                                        Configuration &configuration,
                                                        Diagnostics &diagnostics) {
  auto finder = std::make_unique<SimpleTestFinder>();
  auto runner = std::make_unique<NativeTestRunner>(diagnostics, toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework TestFrameworkFactory::cppuTestFramework(Toolchain &toolchain,
                                                        Configuration &configuration,
                                                        Diagnostics &diagnostics) {
  auto finder = std::make_unique<CppUTestFinder>();
  auto runner = std::make_unique<NativeTestRunner>(diagnostics, toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework TestFrameworkFactory::googleTestFramework(Toolchain &toolchain,
                                                        Configuration &configuration,
                                                        Diagnostics &diagnostics) {
  auto finder = std::make_unique<GoogleTestFinder>();
  auto runner = std::make_unique<NativeTestRunner>(diagnostics, toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

TestFramework TestFrameworkFactory::customTestFramework(Toolchain &toolchain,
                                                        Configuration &configuration,
                                                        Diagnostics &diagnostics) {
  auto finder = std::make_unique<CustomTestFinder>(configuration.customTests);
  auto runner = std::make_unique<NativeTestRunner>(diagnostics, toolchain.mangler());
  return TestFramework(std::move(finder), std::move(runner));
}

std::vector<std::pair<std::string, std::string>> TestFrameworkFactory::commandLineOptions() {
  std::vector<std::pair<std::string, std::string>> options(
      {
        std::make_pair("BoostTest", "Boost.Test Framework"),
        std::make_pair("GoogleTest", "Google Test Framework"),
        std::make_pair("CustomTest", "Custom Test Framework"),
        std::make_pair("CppUTest", "CppUTest Framework"),
        std::make_pair("SimpleTest", "Simple Test (For internal usage only)"),
      });

  return options;
}
