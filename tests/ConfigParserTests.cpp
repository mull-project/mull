#include <thread>

#include "ConfigParserTestFixture.h"
#include "mull/Mutators/CXX/ArithmeticMutators.h"
#include "mull/Mutators/MathDivMutator.h"
#include "mull/Mutators/MathMulMutator.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/Mutators/ScalarValueMutator.h"

TEST_F(ConfigParserTestFixture, loadConfig_bitcodeFileList_unspecified) {
  const char *configYAML = "";

  configWithYamlContent(configYAML);

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);
}

TEST_F(ConfigParserTestFixture, loadConfig_bitcodeFileList_nonExistingFile) {
  const char *configYAML =
      "bitcode_file_list: /tmp/non-existing-file-12345.txt\n";
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
  fs << "# baz.bc" << std::endl;

  const char *configYAML = "bitcode_file_list: /tmp/bitcode_file_list.txt\n";
  configWithYamlContent(configYAML);

  ASSERT_EQ(2U, config.getBitcodePaths().size());
  ASSERT_EQ("foo.bc", *(config.getBitcodePaths().begin()));
  ASSERT_EQ("bar.bc", *(config.getBitcodePaths().end() - 1));

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 0U);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_dynamicLibraryFileList_nonExistingFile) {
  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";

  std::ofstream bitcodeFile(bitcodeFileList);

  if (!bitcodeFile) {
    std::cerr << "Cannot open the output file." << std::endl;
    ASSERT_FALSE(true);
  }

  bitcodeFile << "foo.bc" << std::endl;
  bitcodeFile << "bar.bc" << std::endl;
  bitcodeFile << "# baz.bc" << std::endl;

  const char *configYAML =
      "bitcode_file_list: "
      "/tmp/bitcode_file_list.txt\ndynamic_library_file_list: "
      "/tmp/non-existing-file-12345.txt\n";
  configWithYamlContent(configYAML);

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_bitcodeFileList_dynamicLibraryFileList) {
  const std::string bitcodeFileList = "/tmp/bitcode_file_list.txt";
  const std::string dynamicLibraryFileList =
      "/tmp/dynamic_library_file_list.txt";

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
  bitcodeFile << "# baz.bc" << std::endl;

  dynamicLibraryFile << "sqlite3.dylib" << std::endl;
  dynamicLibraryFile << "libz.dylib" << std::endl;

  const char *configYAML =
      "bitcode_file_list: "
      "/tmp/bitcode_file_list.txt\ndynamic_library_file_list: "
      "/tmp/dynamic_library_file_list.txt";
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
  objectFileStream << "# baz.o" << std::endl;

  const char *configYAML = "bitcode_file_list: /tmp/bitcode_file.list\n"
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

TEST_F(ConfigParserTestFixture, loadConfig_FailFast_Unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.failFastModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_FailFast_Enabled) {
  configWithYamlContent("fail_fast: enabled\n");
  ASSERT_TRUE(config.failFastModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_FailFast_Disabled) {
  configWithYamlContent("fail_fast: disabled\n");
  ASSERT_FALSE(config.failFastModeEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_Diagnostics_Unspecified) {
  configWithYamlContent("");
  ASSERT_EQ(config.getDiagnostics(), Diagnostics::None);
}

TEST_F(ConfigParserTestFixture, loadConfig_Diagnostics_Survived) {
  configWithYamlContent("diagnostics: survived\n");
  ASSERT_EQ(config.getDiagnostics(), Diagnostics::Survived);
}

TEST_F(ConfigParserTestFixture, loadConfig_Diagnostics_Killed) {
  configWithYamlContent("diagnostics: killed\n");
  ASSERT_EQ(config.getDiagnostics(), Diagnostics::Killed);
}

TEST_F(ConfigParserTestFixture, loadConfig_Diagnostics_All) {
  configWithYamlContent("diagnostics: all\n");
  ASSERT_EQ(config.getDiagnostics(), Diagnostics::All);
}

TEST_F(ConfigParserTestFixture, loadConfig_Diagnostics_None) {
  configWithYamlContent("diagnostics: none\n");
  ASSERT_EQ(config.getDiagnostics(), Diagnostics::None);
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_Unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.cachingEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_True) {
  configWithYamlContent("use_cache: true\n");
  ASSERT_TRUE(config.cachingEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_Yes) {
  configWithYamlContent("use_cache: yes\n");
  ASSERT_TRUE(config.cachingEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_False) {
  configWithYamlContent("use_cache: false\n");
  ASSERT_FALSE(config.cachingEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_UseCache_No) {
  configWithYamlContent("use_cache: no\n");
  ASSERT_FALSE(config.cachingEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_Unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.shouldEmitDebugInfo());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_True) {
  configWithYamlContent("emit_debug_info: true\n");
  ASSERT_TRUE(config.shouldEmitDebugInfo());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_Yes) {
  configWithYamlContent("emit_debug_info: yes\n");
  ASSERT_TRUE(config.shouldEmitDebugInfo());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_False) {
  configWithYamlContent("emit_debug_info: false\n");
  ASSERT_FALSE(config.shouldEmitDebugInfo());
}

TEST_F(ConfigParserTestFixture, loadConfig_emitDebugInfo_No) {
  configWithYamlContent("emit_debug_info: no\n");
  ASSERT_FALSE(config.shouldEmitDebugInfo());
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

TEST_F(ConfigParserTestFixture, loadConfig_Mutators_Unspecified) {
  const char *configYAML = "";
  configWithYamlContent(configYAML);

  auto mutators = config.getMutators();
  ASSERT_EQ(0U, mutators.size());
}

TEST_F(ConfigParserTestFixture, loadConfig_Mutators_SpecificValue) {
  const char *configYAML = R"YAML(
mutators:
  - negate_mutator
  - cxx_arithmetic_sub_to_add
  - math_mul_mutator
  - math_div_mutator
  - scalar_value_mutator
  - replace_call_mutator
  )YAML";
  configWithYamlContent(configYAML);

  auto mutators = config.getMutators();
  ASSERT_EQ(6U, mutators.size());
  size_t index = 0;
  ASSERT_EQ(NegateConditionMutator::ID, mutators[index++]);
  ASSERT_EQ(cxx::SubToAdd::ID, mutators[index++]);
  ASSERT_EQ(MathMulMutator::ID, mutators[index++]);
  ASSERT_EQ(MathDivMutator::ID, mutators[index++]);
  ASSERT_EQ(ScalarValueMutator::ID, mutators[index++]);
  ASSERT_EQ(ReplaceCallMutator::ID, mutators[index++]);
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

TEST_F(ConfigParserTestFixture, loadConfig_junkDetectionConfig_unspecified) {
  configWithYamlContent("");
  ASSERT_FALSE(config.junkDetectionEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_junkDetectionConfig_specified) {
  const char *configYAML = R"YAML(
junk_detection:
  detector: all
  enabled: yes
  )YAML";
  configWithYamlContent(configYAML);
  ASSERT_TRUE(config.junkDetectionEnabled());
}

TEST_F(ConfigParserTestFixture, loadConfig_reporters) {
  const char *configYAML = R"YAML(
reporters:
  - sqlite
  - cli
  )YAML";
  configWithYamlContent(configYAML);

  auto reporters = config.getReporters();
  ASSERT_EQ(2U, reporters.size());
  ASSERT_EQ("sqlite", reporters[0]);
  ASSERT_EQ("cli", reporters[1]);
}

TEST_F(ConfigParserTestFixture, loadConfig_parallelization_empty) {
  const char *configYAML = R"YAML(
reporters:
  - time
)YAML";
  configWithYamlContent(configYAML);

  int availableThreads = std::thread::hardware_concurrency();
  auto parallelization = config.parallelization();

  ASSERT_EQ(availableThreads, parallelization.workers);
  ASSERT_EQ(availableThreads, parallelization.mutantExecutionWorkers);
  ASSERT_EQ(availableThreads, parallelization.testExecutionWorkers);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_parallelization_workers_value_propagates) {
  const char *configYAML = R"YAML(
parallelization:
  workers: 33
  )YAML";
  configWithYamlContent(configYAML);

  int availableThreads = 33;
  auto parallelization = config.parallelization();

  ASSERT_EQ(availableThreads, parallelization.workers);
  ASSERT_EQ(availableThreads, parallelization.mutantExecutionWorkers);
  ASSERT_EQ(availableThreads, parallelization.testExecutionWorkers);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_parallelization_local_value_overrides) {
  const char *configYAML = R"YAML(
parallelization:
  workers: 33
  test_execution_workers: 14
  )YAML";
  configWithYamlContent(configYAML);

  int availableThreads = 33;
  auto parallelization = config.parallelization();

  ASSERT_EQ(availableThreads, parallelization.workers);
  ASSERT_EQ(availableThreads, parallelization.mutantExecutionWorkers);
  ASSERT_EQ(14, parallelization.testExecutionWorkers);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_parallelization_all_local_values_override) {
  const char *configYAML = R"YAML(
parallelization:
  workers: 33
  test_execution_workers: 14
  mutant_execution_workers: 12
  )YAML";
  configWithYamlContent(configYAML);

  auto parallelization = config.parallelization();

  ASSERT_EQ(33, parallelization.workers);
  ASSERT_EQ(12, parallelization.mutantExecutionWorkers);
  ASSERT_EQ(14, parallelization.testExecutionWorkers);
}

TEST_F(ConfigParserTestFixture, loadConfig_parallelization_local_values_only) {
  const char *configYAML = R"YAML(
parallelization:
  test_execution_workers: 14
  mutant_execution_workers: 12
  )YAML";
  configWithYamlContent(configYAML);

  auto parallelization = config.parallelization();

  ASSERT_EQ(12, parallelization.mutantExecutionWorkers);
  ASSERT_EQ(14, parallelization.testExecutionWorkers);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_getMutationTestingElementsReportPath_Unspecified) {
  configWithAMinimalValidYamlContent("");
  ASSERT_EQ("", config.getMutationTestingElementsReportPath());

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 0U);
}

TEST_F(ConfigParserTestFixture,
       loadConfig_getMutationTestingElementsReportPath_SpecificValue_ValidDir) {
  configWithAMinimalValidYamlContent(
      "mutation_testing_elements_report_path: "
      "/tmp/mull.mutation_testing_elements.json\n");
  ASSERT_EQ("/tmp/mull.mutation_testing_elements.json",
            config.getMutationTestingElementsReportPath());

  auto errors = config.validate();
  for (auto &error : errors) {
    errs() << error << "\n";
  }

  ASSERT_EQ(errors.size(), 0U);
}

TEST_F(
    ConfigParserTestFixture,
    loadConfig_getMutationTestingElementsReportPath_SpecificValue_InvalidDir) {
  configWithAMinimalValidYamlContent(
      "mutation_testing_elements_report_path: "
      "/invalid-dir/mull.mutation_testing_elements.json\n");

  auto errors = config.validate();
  ASSERT_EQ(errors.size(), 1U);

  std::string expectedError =
      "error: could not create a directory for "
      "mutation_testing_elements_reporter_path:\n/invalid-dir"
      "\nerror: Permission denied";
  ASSERT_EQ(errors[0], expectedError);
}
