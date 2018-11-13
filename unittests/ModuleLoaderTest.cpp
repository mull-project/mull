#include "gtest/gtest.h"

#include "ModuleLoader.h"
#include "FixturePaths.h"
#include "Config/Configuration.h"

using namespace mull;
using namespace llvm;

TEST(ModuleLoaderTest, loadModuleFromBitcodeListFile) {
  Configuration configuration;
  ModuleLoader loader;
  std::string bitcodeFile = fixtures::hardcode_fixture_simple_test_tester_module_bc_path();

  std::vector<std::string> bitcodePaths = { bitcodeFile };
  std::vector<std::unique_ptr<MullModule>> modules =
      loader.loadModulesFromBitcodeFileList(bitcodePaths, configuration);

  ASSERT_EQ(modules.size(), 1U);
}
