#include "ConfigParserTestFixture.h"

#include "MutationOperators/MathAddMutationOperator.h"
#include "MutationOperators/MathDivMutationOperator.h"
#include "MutationOperators/MathMulMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "MutationOperators/ReplaceCallMutationOperator.h"
#include "MutationOperators/ScalarValueMutationOperator.h"

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

TEST_F(ConfigParserTestFixture, loadConfig_objectFileList) {
  const std::string bitcodeFileList = "/tmp/bitcode_file.list";
  const std::string objectFiles = "/tmp/object_files.list";

  std::ofstream bitcodeFileStream(bitcodeFileList);
  if (!bitcodeFileStream) {
    std::cerr << "Cannot open the output file." << std::endl;
    ASSERT_FALSE(true);
  }

  std::ofstream objectFileStream(objectFiles);

  if (!objectFileStream) {
    std::cerr << "Cannot open the output file." << std::endl;
    ASSERT_FALSE(true);
  }

  objectFileStream << "foo.o" << std::endl;
  objectFileStream << "bar.o" << std::endl;
  objectFileStream << "# baz.o"  << std::endl;

  const char *configYAML =
    "bitcode_file_list: /tmp/bitcode_file.list\n"
    "object_file_list: /tmp/object_files.list";
  configWithYamlContent(configYAML);

  ASSERT_EQ("/tmp/object_files.list", config.getObjectFileList());

  ASSERT_EQ(2U, config.getObjectFilesPaths().size());
  ASSERT_EQ("foo.o", *(config.getObjectFilesPaths().begin()));
  ASSERT_EQ("bar.o", *(config.getObjectFilesPaths().end() - 1));

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 0U);
}

TEST_F(ConfigParserTestFixture, loadConfig_objectFileList_nonExistingFile) {
  const std::string bitcodeFileList = "/tmp/bitcode_file.list";

  std::ofstream bitcodeFileStream(bitcodeFileList);
  if (!bitcodeFileStream) {
    std::cerr << "Cannot open the output file." << std::endl;
    ASSERT_FALSE(true);
  }

  const char *configYAML =
    "bitcode_file_list: /tmp/bitcode_file.list\n"
    "object_file_list: /tmp/object_files.list.does.not.exist";

  configWithYamlContent(configYAML);

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_True) {
  configWithYamlContent("fork: true\n");
  ASSERT_EQ(true, config.forkEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_Enabled) {
  configWithYamlContent("fork: enabled\n");
  ASSERT_EQ(true, config.forkEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_False) {
  configWithYamlContent("fork: false\n");
  ASSERT_EQ(false, config.forkEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_Disabled) {
  configWithYamlContent("fork: disabled\n");
  ASSERT_EQ(false, config.forkEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_Fork_Unspecified) {
  configWithYamlContent("");
  ASSERT_EQ(true, config.forkEnabled());
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
  ASSERT_FALSE(config.dryRunModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_DryRun_True) {
  configWithYamlContent("dry_run: true\n");
  ASSERT_TRUE(config.dryRunModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_DryRun_Enabled) {
  configWithYamlContent("dry_run: enabled\n");
  ASSERT_TRUE(config.dryRunModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_DryRun_False) {
  configWithYamlContent("dry_run: false\n");
  ASSERT_FALSE(config.dryRunModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_DryRun_Disabled) {
  configWithYamlContent("dry_run: disabled\n");
  ASSERT_FALSE(config.dryRunModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_Unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.getUseCache());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_SpecificValue) {
  configWithYamlContent("use_cache: true\n");
  ASSERT_TRUE(config.getUseCache());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_Unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.shouldEmitDebugInfo());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_SpecificValue) {
  configWithYamlContent("emit_debug_info: true\n");
  ASSERT_TRUE(config.shouldEmitDebugInfo());
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
  - math_add_mutation_operator
  - negate_mutation_operator
  - math_sub_mutation_operator
  - math_mul_mutation_operator
  - math_div_mutation_operator
  - scalar_value_mutation_operator
  - replace_call_mutation_operator
  )YAML";
  configWithYamlContent(configYAML);
  
  auto mutationOperators = config.getMutationOperators();
  ASSERT_EQ(7U, mutationOperators.size());
  ASSERT_EQ(MathAddMutationOperator::ID, mutationOperators[0]);
  ASSERT_EQ(NegateConditionMutationOperator::ID, mutationOperators[1]);
  ASSERT_EQ(MathSubMutationOperator::ID, mutationOperators[2]);
  ASSERT_EQ(MathMulMutationOperator::ID, mutationOperators[3]);
  ASSERT_EQ(MathDivMutationOperator::ID, mutationOperators[4]);
  ASSERT_EQ(ScalarValueMutationOperator::ID, mutationOperators[5]);
  ASSERT_EQ(ReplaceCallMutationOperator::ID, mutationOperators[6]);
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
