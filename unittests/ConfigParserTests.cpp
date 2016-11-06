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

    ASSERT_EQ(2U, Cfg->getBitcodePaths().size());
    ASSERT_EQ("foo.bc", *(Cfg->getBitcodePaths().begin()));
    ASSERT_EQ("bar.bc", *(Cfg->getBitcodePaths().end() - 1));
    ASSERT_EQ("bar.bc", *(Cfg->getBitcodePaths().end() - 1));
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

TEST(ConfigParser, loadConfig_Timeout_Unspecified) {
  SourceMgr SM;

  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Stream Stream("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(MutangDefaultTimeout, Cfg->getTimeout());
}

TEST(ConfigParser, loadConfig_Timeout_SpecificValue) {
  SourceMgr SM;
  yaml::Stream Stream("timeout: 15\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(15, Cfg->getTimeout());
}

TEST(ConfigParser, loadConfig_DryRun_Unspecified) {
  SourceMgr SM;

  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Stream Stream("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_FALSE(Cfg->isDryRun());
}

TEST(ConfigParser, loadConfig_DryRun_SpecificValue) {
  ConfigParser Parser;
  SourceMgr SM;
  yaml::Stream Stream("dryRun: true\n", SM);
  auto Cfg = Parser.loadConfig(Stream);
  ASSERT_TRUE(Cfg->isDryRun());
}

TEST(ConfigParser, loadConfig_UseCache_Unspecified) {
  SourceMgr SM;

  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Stream Stream("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);
  ASSERT_TRUE(Cfg->getUseCache());
}

TEST(ConfigParser, loadConfig_UseCache_SpecificValue) {
  SourceMgr SM;
  yaml::Stream Stream("use_cache: false\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);
  ASSERT_FALSE(Cfg->getUseCache());
}

TEST(ConfigParser, loadConfig_MaxDistance_Unspecified) {
  SourceMgr SM;

  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Stream Stream("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(128, Cfg->getMaxDistance());
}

TEST(ConfigParser, loadConfig_MaxDistance_SpecificValue) {
  SourceMgr SM;
  yaml::Stream Stream("maxDistance: 3\n", SM);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Stream);

  ASSERT_EQ(3, Cfg->getMaxDistance());
}
