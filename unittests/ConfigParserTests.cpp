#include "ConfigParser.h"
#include "Config.h"

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include "gtest/gtest.h"

using namespace mull;
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
  yaml::Input Input("fork: true\n");

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
  yaml::Input Input("");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(true, Cfg.getFork());
}

TEST(ConfigParser, loadConfig_Timeout_Unspecified) {
  yaml::Input Input("");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(MullDefaultTimeout, Cfg.getTimeout());
}

TEST(ConfigParser, loadConfig_Timeout_SpecificValue) {
  yaml::Input Input("timeout: 15\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ(15, Cfg.getTimeout());
}

TEST(ConfigParser, loadConfig_DryRun_Unspecified) {
  yaml::Input Input("");

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
  yaml::Input Input("");

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
  yaml::Input Input("");

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
  yaml::Input Input("");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ("/tmp/mull_cache", Cfg.getCacheDirectory());
}

TEST(ConfigParser, loadConfig_CacheDirectory_SpecificValue) {
  yaml::Input Input("cache_directory: /var/tmp\n");

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  ASSERT_EQ("/var/tmp", Cfg.getCacheDirectory());
}

TEST(ConfigParser, loadConfig_MutationOperators_SpecificValue) {
  const char *configYAML = R"YAML(
mutation_operators:
- add_mutation_operator
- negate_mutation_operator
)YAML";

  yaml::Input Input(configYAML);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  auto mutationOperators = Cfg.getMutationOperators();
  ASSERT_EQ(2U, mutationOperators.size());
  ASSERT_EQ(AddMutationOperator::ID, mutationOperators[0]);
  ASSERT_EQ(NegateConditionMutationOperator::ID, mutationOperators[1]);
}

TEST(ConfigParser, loadConfig_MutationOperators_Unspecified) {
  const char *configYAML = "";

  yaml::Input Input(configYAML);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(Input);

  auto mutationOperators = Cfg.getMutationOperators();
  ASSERT_EQ(0U, mutationOperators.size());
}
