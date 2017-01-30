#include "ConfigParserTestFixture.h"

TEST_F(ConfigParserTestFixture, loadConfig_BitcodeFiles) {
  const char *configYAML = "bitcode_files:\n"
                           "  - foo.bc\n"
                           "  - bar.bc\n";
  configWithYamlContent(configYAML);
  
  ASSERT_EQ(2U, config.getBitcodePaths().size());
  ASSERT_EQ("foo.bc", *(config.getBitcodePaths().begin()));
  ASSERT_EQ("bar.bc", *(config.getBitcodePaths().end() - 1));
  ASSERT_EQ("bar.bc", *(config.getBitcodePaths().end() - 1));
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_True) {
  configWithYamlContent("fork: true\n");
  ASSERT_EQ(true, config.getFork());
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_False) {
  configWithYamlContent("fork: false\n");
  ASSERT_EQ(false, config.getFork());
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_Unspecified) {
  configWithYamlContent("");
  ASSERT_EQ(true, config.getFork());
}

TEST_F(ConfigParserTestFixture, loadConfig_Timeout_Unspecified) {
  configWithYamlContent("");
  ASSERT_EQ(MullDefaultTimeoutMilliseconds, config.getTimeout());
}

TEST_F(ConfigParserTestFixture, loadConfig_Timeout_SpecificValue) {
  configWithYamlContent("timeout: 15\n");
  ASSERT_EQ(15, config.getTimeout());
}

TEST_F(ConfigParserTestFixture, loadConfig_DryRun_Unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.isDryRun());
}

TEST_F(ConfigParserTestFixture, loadConfig_DryRun_SpecificValue) {
  configWithYamlContent("dry_run: true\n");
  ASSERT_TRUE(config.isDryRun());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_Unspecified) {
  configWithYamlContent("");
  ASSERT_TRUE(config.getUseCache());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_SpecificValue) {
  configWithYamlContent("use_cache: false\n");
  ASSERT_FALSE(config.getUseCache());
}

TEST_F(ConfigParserTestFixture, loadConfig_MaxDistance_Unspecified) {
  configWithYamlContent("fork: true\n");
  ASSERT_EQ(128, config.getMaxDistance());
}

TEST_F(ConfigParserTestFixture, loadConfig_MaxDistance_SpecificValue) {
  configWithYamlContent("max_distance: 3\n");
  ASSERT_EQ(3, config.getMaxDistance());
}

TEST_F(ConfigParserTestFixture, loadConfig_CacheDirectory_Unspecified) {
  configWithYamlContent("");
  ASSERT_EQ("/tmp/mull_cache", config.getCacheDirectory());
}

TEST_F(ConfigParserTestFixture, loadConfig_CacheDirectory_SpecificValue) {
  configWithYamlContent("cache_directory: /var/tmp\n");
  ASSERT_EQ("/var/tmp", config.getCacheDirectory());
}

TEST_F(ConfigParserTestFixture, loadConfig_MutationOperators_SpecificValue) {
  const char *configYAML = R"YAML(
mutation_operators:
  - add_mutation_operator
  - negate_mutation_operator
  )YAML";
  configWithYamlContent(configYAML);
  
  auto mutationOperators = config.getMutationOperators();
  ASSERT_EQ(2U, mutationOperators.size());
  ASSERT_EQ(AddMutationOperator::ID, mutationOperators[0]);
  ASSERT_EQ(NegateConditionMutationOperator::ID, mutationOperators[1]);
}

TEST_F(ConfigParserTestFixture, loadConfig_MutationOperators_Unspecified) {
  configWithYamlContent("");
  auto mutationOperators = config.getMutationOperators();
  ASSERT_EQ(0U, mutationOperators.size());
}
