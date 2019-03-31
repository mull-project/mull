#include "gtest/gtest.h"

#include "FixturePaths.h"
#include "mull/Config/Configuration.h"
#include "mull/ModuleLoader.h"

using namespace mull;
using namespace llvm;

TEST(ModuleLoaderTest, loadModuleFromBitcodeListFile) {
  Configuration configuration;
  configuration.bitcodePaths = {
      fixtures::hardcode_fixture_simple_test_tester_module_bc_path()};

  ModuleLoader loader;
  std::vector<std::unique_ptr<MullModule>> modules =
      loader.loadModules(configuration);

  ASSERT_EQ(modules.size(), 1U);
}
