#include "ConfigParserTestFixture.h"

TEST_F(ConfigParserTestFixture, loadConfig_bitcodeFileList_unspecified) {
  const char *configYAML = "";

  configWithYamlContent(configYAML);

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);
}

TEST_F(ConfigParserTestFixture, loadConfig_bitcodeFileList_nonExistingFile) {
  const char *configYAML = "bitcode_file_list: /tmp/non-existing-file-12345.txt\n";
  configWithYamlContent(configYAML);

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);
}

TEST_F(ConfigParserTestFixture, loadConfig_bitcodeFileList) {
  static std::string BitcodeFileList = "/tmp/bitcode_file_list.txt";

  std::ofstream fs(BitcodeFileList);

  if (!fs) {
    std::cerr << "Cannot open the output file." << std::endl;
    ASSERT_FALSE(true);
  }

  fs << "foo.bc" << std::endl;
  fs << "bar.bc" << std::endl;
  fs << "# baz.bc"  << std::endl;

  const char *configYAML = "bitcode_file_list: /tmp/bitcode_file_list.txt\n";
  configWithYamlContent(configYAML);

  ASSERT_EQ(2U, config.getBitcodePaths().size());
  ASSERT_EQ("foo.bc", *(config.getBitcodePaths().begin()));
  ASSERT_EQ("bar.bc", *(config.getBitcodePaths().end() - 1));

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 0U);
}

TEST_F(ConfigParserTestFixture, loadConfig_dynamicLibraryFileList_nonExistingFile) {
  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  
  std::ofstream bitcodeFile(bitcodeFileList);
  
  if (!bitcodeFile) {
    std::cerr << "Cannot open the output file." << std::endl;
    ASSERT_FALSE(true);
  }
  
  bitcodeFile << "foo.bc" << std::endl;
  bitcodeFile << "bar.bc" << std::endl;
  bitcodeFile << "# baz.bc"  << std::endl;
  
  const char *configYAML = "bitcode_file_list: /tmp/bitcode_file_list.txt\ndynamic_library_file_list: /tmp/non-existing-file-12345.txt\n";
  configWithYamlContent(configYAML);
  
  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);
}

TEST_F(ConfigParserTestFixture, loadConfig_bitcodeFileList_dynamicLibraryFileList) {
  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList = "/tmp/dynamic_library_file_list.txt";
  
  std::ofstream bitcodeFile(bitcodeFileList);
  std::ofstream dynamicLibraryFile(dynamicLibraryFileList);
  
  if (!bitcodeFile) {
    std::cerr << "Cannot open bitcode file." << std::endl;
    ASSERT_FALSE(true);
  }
  
  if (!dynamicLibraryFile) {
    std::cerr << "Cannot open dynamic library file." << std::endl;
    ASSERT_FALSE(true);
  }
  
  bitcodeFile << "foo.bc" << std::endl;
  bitcodeFile << "bar.bc" << std::endl;
  bitcodeFile << "# baz.bc"  << std::endl;
  
  dynamicLibraryFile << "sqlite3.dylib" << std::endl;
  dynamicLibraryFile << "libz.dylib" << std::endl;
  
  const char *configYAML = "bitcode_file_list: /tmp/bitcode_file_list.txt\ndynamic_library_file_list: /tmp/dynamic_library_file_list.txt";
  configWithYamlContent(configYAML);
  
  ASSERT_EQ(2U, config.getBitcodePaths().size());
  ASSERT_EQ("foo.bc", *(config.getBitcodePaths().begin()));
  ASSERT_EQ("bar.bc", *(config.getBitcodePaths().end() - 1));
  
  ASSERT_EQ(2U, config.getDynamicLibrariesPaths().size());
  ASSERT_EQ("sqlite3.dylib", *(config.getDynamicLibrariesPaths().begin()));
  ASSERT_EQ("libz.dylib", *(config.getDynamicLibrariesPaths().end() - 1));
  
  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 0U);
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

TEST_F(ConfigParserTestFixture, loadConfig_MutationOperators_Unspecified) {
  const char *configYAML = "";
  configWithYamlContent(configYAML);

  auto mutationOperators = config.getMutationOperators();
  ASSERT_EQ(0U, mutationOperators.size());
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

TEST_F(ConfigParserTestFixture, loadConfig_tests_SpecificValue) {
  const char *configYAML = R"YAML(
tests:
  - test1
  - test2
)YAML";
  configWithYamlContent(configYAML);

  auto tests = config.getTests();
  ASSERT_EQ(2U, tests.size());
  ASSERT_EQ("test1", tests[0]);
  ASSERT_EQ("test2", tests[1]);
}

TEST_F(ConfigParserTestFixture, loadConfig_ProjectName_Specified) {
  const char *configYAML = "project_name: FooBar";
  configWithYamlContent(configYAML);
  ASSERT_EQ("FooBar", config.getProjectName());
}

TEST_F(ConfigParserTestFixture, loadConfig_ProjectName_Unspecified) {
  configWithYamlContent("");
  ASSERT_EQ("", config.getProjectName());
}

TEST_F(ConfigParserTestFixture, loadConfig_excludeLocations_unspecified) {
  configWithYamlContent("");
  ASSERT_EQ(0U, config.getExcludeLocations().size());
}

TEST_F(ConfigParserTestFixture, loadConfig_excludeLocations_specified) {
  const char *configYAML = R"YAML(
exclude_locations:
  - include/c++/v1
  - llvm/include
  )YAML";
  configWithYamlContent(configYAML);

  auto excludeLocations = config.getExcludeLocations();
  ASSERT_EQ(2U, excludeLocations.size());
  ASSERT_EQ("include/c++/v1", excludeLocations[0]);
  ASSERT_EQ("llvm/include", excludeLocations[1]);
}
