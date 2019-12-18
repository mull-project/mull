#include "gtest/gtest.h"
#include <mull/Diagnostics/Diagnostics.h>

#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"

using namespace mull;
using namespace llvm;

TEST(BitcodeLoaderTest, loadBitcodeFromBitcodeListFile) {
  Configuration configuration;
  configuration.bitcodePaths = { fixtures::hardcode_fixture_simple_test_tester_module_bc_path() };
  Diagnostics diagnostics;
  BitcodeLoader loader;
  std::vector<std::unique_ptr<Bitcode>> bitcode = loader.loadBitcode(configuration, diagnostics);

  ASSERT_EQ(bitcode.size(), 1U);
}
