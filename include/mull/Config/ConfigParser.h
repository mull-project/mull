#pragma once

#include "mull/Config/RawConfig.h"

#include <llvm/Support/YAMLTraits.h>

#if LLVM_VERSION_MAJOR < 5

#ifndef YAML_STRING_SEQUENCE
#define YAML_STRING_SEQUENCE
LLVM_YAML_IS_SEQUENCE_VECTOR(std::string)
#endif

#endif

namespace llvm {
namespace yaml {
class Input;

template <> struct SequenceTraits<std::vector<mull::CustomTestDefinition>> {
  static size_t size(IO &io, std::vector<mull::CustomTestDefinition> &list) {
    return list.size();
  }
  static mull::CustomTestDefinition &
  element(IO &io, std::vector<mull::CustomTestDefinition> &list, size_t index) {
    mull::CustomTestDefinition t;
    list.push_back(t);
    return list.back();
  }
};

template <> struct MappingTraits<mull::CustomTestDefinition> {
  static void mapping(IO &io, mull::CustomTestDefinition &test) {
    io.mapRequired("name", test.testName);
    io.mapRequired("method", test.methodName);
    io.mapOptional("program", test.programName);
    io.mapRequired("arguments", test.callArguments);
  }
};

template <> struct MappingTraits<mull::JunkDetectionConfig> {
  static void mapping(IO &io, mull::JunkDetectionConfig &config) {
    io.mapRequired("detector", config.detectorName);
    io.mapOptional("enabled", config.toggle);
    io.mapOptional("cxx_compdb_dir", config.cxxCompilationDatabasePath);
    io.mapOptional("cxx_compilation_flags", config.cxxCompilationFlags);
  }
};

template <>
struct ScalarEnumerationTraits<mull::JunkDetectionConfig::JunkDetectionToggle> {
  static void
  enumeration(IO &io, mull::JunkDetectionConfig::JunkDetectionToggle &toggle) {
    io.enumCase(toggle, "true",
                mull::JunkDetectionConfig::JunkDetectionToggle::Enabled);
    io.enumCase(toggle, "yes",
                mull::JunkDetectionConfig::JunkDetectionToggle::Enabled);
    io.enumCase(toggle, "enabled",
                mull::JunkDetectionConfig::JunkDetectionToggle::Enabled);
    io.enumCase(toggle, "false",
                mull::JunkDetectionConfig::JunkDetectionToggle::Disabled);
    io.enumCase(toggle, "no",
                mull::JunkDetectionConfig::JunkDetectionToggle::Disabled);
    io.enumCase(toggle, "disabled",
                mull::JunkDetectionConfig::JunkDetectionToggle::Disabled);
  }
};

template <> struct ScalarEnumerationTraits<mull::RawConfig::Fork> {
  static void enumeration(IO &io, mull::RawConfig::Fork &fork) {
    io.enumCase(fork, "true", mull::RawConfig::Fork::Enabled);
    io.enumCase(fork, "enabled", mull::RawConfig::Fork::Enabled);
    io.enumCase(fork, "false", mull::RawConfig::Fork::Disabled);
    io.enumCase(fork, "disabled", mull::RawConfig::Fork::Disabled);
  }
};

template <> struct ScalarEnumerationTraits<mull::RawConfig::DryRunMode> {
  static void enumeration(IO &io, mull::RawConfig::DryRunMode &value) {
    io.enumCase(value, "true", mull::RawConfig::DryRunMode::Enabled);
    io.enumCase(value, "enabled", mull::RawConfig::DryRunMode::Enabled);
    io.enumCase(value, "false", mull::RawConfig::DryRunMode::Disabled);
    io.enumCase(value, "disabled", mull::RawConfig::DryRunMode::Disabled);
  }
};

template <> struct ScalarEnumerationTraits<mull::RawConfig::FailFastMode> {
  static void enumeration(IO &io, mull::RawConfig::FailFastMode &value) {
    io.enumCase(value, "enabled", mull::RawConfig::FailFastMode::Enabled);
    io.enumCase(value, "disabled", mull::RawConfig::FailFastMode::Disabled);
  }
};

template <> struct ScalarEnumerationTraits<mull::RawConfig::UseCache> {
  static void enumeration(IO &io, mull::RawConfig::UseCache &value) {
    io.enumCase(value, "true", mull::RawConfig::UseCache::Yes);
    io.enumCase(value, "yes", mull::RawConfig::UseCache::Yes);
    io.enumCase(value, "false", mull::RawConfig::UseCache::No);
    io.enumCase(value, "no", mull::RawConfig::UseCache::No);
  }
};

template <> struct ScalarEnumerationTraits<mull::RawConfig::EmitDebugInfo> {
  static void enumeration(IO &io, mull::RawConfig::EmitDebugInfo &value) {
    io.enumCase(value, "true", mull::RawConfig::EmitDebugInfo::Yes);
    io.enumCase(value, "yes", mull::RawConfig::EmitDebugInfo::Yes);
    io.enumCase(value, "false", mull::RawConfig::EmitDebugInfo::No);
    io.enumCase(value, "no", mull::RawConfig::EmitDebugInfo::No);
  }
};

template <> struct ScalarEnumerationTraits<mull::Diagnostics> {
  static void enumeration(IO &io, mull::Diagnostics &value) {
    io.enumCase(value, "none", mull::Diagnostics::None);
    io.enumCase(value, "survived", mull::Diagnostics::Survived);
    io.enumCase(value, "killed", mull::Diagnostics::Killed);
    io.enumCase(value, "all", mull::Diagnostics::All);
  }
};

template <> struct MappingTraits<mull::ParallelizationConfig> {
  static void mapping(IO &io, mull::ParallelizationConfig &config) {
    io.mapOptional("workers", config.workers);
    io.mapOptional("test_execution_workers", config.testExecutionWorkers);
    io.mapOptional("mutant_execution_workers", config.mutantExecutionWorkers);
  }
};

template <> struct MappingTraits<mull::RawConfig> {
  static void mapping(IO &io, mull::RawConfig &config) {
    io.mapOptional("bitcode_file_list", config.bitcodeFileList);
    io.mapOptional("project_name", config.projectName);
    io.mapOptional("test_framework", config.testFramework);
    io.mapOptional("dynamic_library_file_list", config.dynamicLibraryFileList);
    io.mapOptional("object_file_list", config.objectFileList);
    io.mapOptional("mutators", config.mutators);
    io.mapOptional("reporters", config.reporters);
    io.mapOptional("tests", config.tests);
    io.mapOptional("exclude_locations", config.excludeLocations);
    io.mapOptional("custom_tests", config.customTests);
    io.mapOptional("fork", config.fork);
    io.mapOptional("dry_run", config.dryRun);
    io.mapOptional("fail_fast", config.failFast);
    io.mapOptional("use_cache", config.caching);
    io.mapOptional("emit_debug_info", config.emitDebugInfo);
    io.mapOptional("diagnostics", config.diagnostics);
    io.mapOptional("timeout", config.timeout);
    io.mapOptional("max_distance", config.maxDistance);
    io.mapOptional("cache_directory", config.cacheDirectory);
    io.mapOptional("junk_detection", config.junkDetection);
    io.mapOptional("parallelization", config.parallelizationConfig);
    io.mapOptional("mutation_testing_elements_report_path",
                   config.mutationTestingElementsReportPath);
  }
};
} // namespace yaml
} // namespace llvm

namespace mull {

class ConfigParser {
public:
  RawConfig loadConfig(llvm::yaml::Input &input);
  RawConfig loadConfig(const char *filename);
};
} // namespace mull
