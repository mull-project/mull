#include "ConfigParser.h"
#include "Config.h"

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

TEST(ConfigParser, loadConfig_BitcodeFiles) {
    SourceMgr SM;
    yaml::Stream Stream(
                        "bitcode_files:\n"
                        "  - foo.bc\n"
                        "  - bar.bc\n", SM);

    ConfigParser Parser;
    auto Cfg = Parser.loadConfig(Stream);

    ASSERT_EQ(2U, Cfg->GetBitcodePaths().size());
    ASSERT_EQ("foo.bc", *(Cfg->GetBitcodePaths().begin()));
    ASSERT_EQ("bar.bc", *(Cfg->GetBitcodePaths().end() - 1));
    ASSERT_EQ("bar.bc", *(Cfg->GetBitcodePaths().end() - 1));
}

TEST(ConfigParser, loadConfig_Fork_True) {
  SourceMgr SM;
  yaml::Stream Stream(
                      "fork: true\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(true, Cfg->getFork());
}

TEST(ConfigParser, loadConfig_Fork_False) {
  SourceMgr SM;
  yaml::Stream Stream("fork: false\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(false, Cfg->getFork());
}

TEST(ConfigParser, loadConfig_Fork_Unspecified) {
  SourceMgr SM;

  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Stream Stream("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(true, Cfg->getFork());
}
