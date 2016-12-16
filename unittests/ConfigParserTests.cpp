#include "ConfigParser.h"
#include "Config.h"

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

TEST(ConfigParser, loadConfig_BitcodeFiles) {
    yaml::Input Input(
                        "bitcode_files:\n"
                        "  - foo.bc\n"
                        "  - bar.bc\n");

    ConfigParser Parser;
    auto Cfg = Parser.loadConfig(Input);

    ASSERT_EQ(2U, Cfg.getBitcodePaths().size());
    ASSERT_EQ("foo.bc", *(Cfg.getBitcodePaths().begin()));
    ASSERT_EQ("bar.bc", *(Cfg.getBitcodePaths().end() - 1));
    ASSERT_EQ("bar.bc", *(Cfg.getBitcodePaths().end() - 1));
}

TEST(ConfigParser, loadConfig_Fork_True) {
  yaml::Input Input(
                      "fork: true\n");
  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(true, Cfg.getFork());
}

TEST(ConfigParser, loadConfig_Fork_False) {
  yaml::Input Input("fork: false\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(false, Cfg.getFork());
}

TEST(ConfigParser, loadConfig_Fork_Unspecified) {
  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Input Input("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(true, Cfg.getFork());
}

TEST(ConfigParser, loadConfig_Timeout_Unspecified) {
  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Input Input("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(MutangDefaultTimeout, Cfg.getTimeout());
}

TEST(ConfigParser, loadConfig_Timeout_SpecificValue) {
  yaml::Input Input("timeout: 15\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(15, Cfg.getTimeout());
}

TEST(ConfigParser, loadConfig_DryRun_Unspecified) {
  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Input Input("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_FALSE(Cfg.isDryRun());
}

TEST(ConfigParser, loadConfig_DryRun_SpecificValue) {
  ConfigParser Parser;
  yaml::Input Input("dry_run: true\n");
  auto Cfg = Parser.loadConfig(Input);
  ASSERT_TRUE(Cfg.isDryRun());
}

TEST(ConfigParser, loadConfig_UseCache_Unspecified) {
  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Input Input("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);
  ASSERT_TRUE(Cfg.getUseCache());
}

TEST(ConfigParser, loadConfig_UseCache_SpecificValue) {
  yaml::Input Input("use_cache: false\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);
  ASSERT_FALSE(Cfg.getUseCache());
}

TEST(ConfigParser, loadConfig_MaxDistance_Unspecified) {
  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Input Input("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(128, Cfg.getMaxDistance());
}

TEST(ConfigParser, loadConfig_MaxDistance_SpecificValue) {
  yaml::Input Input("max_distance: 3\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(3, Cfg.getMaxDistance());
}

TEST(ConfigParser, loadConfig_CacheDirectory_Unspecified) {
  /// Surprisingly enough, yaml library crashes on empty string so
  /// providing 'bitcode_files:' with content just to overcome the assert.
  yaml::Input Input("bitcode_files:\n"
                      "  - foo.bc\n"
                      "  - bar.bc\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ("/tmp/mutang_cache", Cfg.getCacheDirectory());
}

TEST(ConfigParser, loadConfig_CacheDirectory_SpecificValue) {
  yaml::Input Input("cache_directory: /var/tmp\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ("/var/tmp", Cfg.getCacheDirectory());
}
