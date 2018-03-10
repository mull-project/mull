#pragma once

#include "Config.h"

#include "llvm/Support/YAMLTraits.h"

#ifndef YAML_STRING_SEQUENCE
#define YAML_STRING_SEQUENCE
LLVM_YAML_IS_SEQUENCE_VECTOR(std::string)
#endif

namespace llvm {
namespace yaml {
class Input;

template <>
struct SequenceTraits<std::vector<mull::CustomTestDefinition>> {
  static size_t size(IO &io, std::vector<mull::CustomTestDefinition> &list) {
    return list.size();
  }
  static mull::CustomTestDefinition &element(IO &io, std::vector<mull::CustomTestDefinition> &list, size_t index) {
    mull::CustomTestDefinition t;
    list.push_back(t);
    return list.back();
  }
};

template <>
struct MappingTraits<mull::CustomTestDefinition> {
  static void mapping(IO &io, mull::CustomTestDefinition &test) {
    io.mapRequired("name", test.testName);
    io.mapRequired("method", test.methodName);
    io.mapOptional("program", test.programName);
    io.mapRequired("arguments", test.callArguments);
  }
};

template <>
struct MappingTraits<mull::JunkDetectionConfig> {
  static void mapping(IO &io, mull::JunkDetectionConfig &config) {
    io.mapRequired("detector", config.detectorName);
    io.mapOptional("enabled", config.toggle);
    io.mapOptional("cxx_compdb_dir", config.cxxCompDBDirectory);
    io.mapOptional("cxx_compilation_flags", config.cxxCompilationFlags);
  }
};

template <>
struct ScalarEnumerationTraits<mull::JunkDetectionConfig::JunkDetectionToggle> {
  static void enumeration(IO &io, mull::JunkDetectionConfig::JunkDetectionToggle &toggle) {
    io.enumCase(toggle, "true",  mull::JunkDetectionConfig::JunkDetectionToggle::Enabled);
    io.enumCase(toggle, "yes",  mull::JunkDetectionConfig::JunkDetectionToggle::Enabled);
    io.enumCase(toggle, "enabled",  mull::JunkDetectionConfig::JunkDetectionToggle::Enabled);
    io.enumCase(toggle, "false",  mull::JunkDetectionConfig::JunkDetectionToggle::Disabled);
    io.enumCase(toggle, "no",  mull::JunkDetectionConfig::JunkDetectionToggle::Disabled);
    io.enumCase(toggle, "disabled",  mull::JunkDetectionConfig::JunkDetectionToggle::Disabled);
  }
};

template <>
struct ScalarEnumerationTraits<mull::Config::Fork> {
  static void enumeration(IO &io, mull::Config::Fork &fork) {
    io.enumCase(fork, "true",  mull::Config::Fork::Enabled);
    io.enumCase(fork, "enabled",  mull::Config::Fork::Enabled);
    io.enumCase(fork, "false",  mull::Config::Fork::Disabled);
    io.enumCase(fork, "disabled",  mull::Config::Fork::Disabled);
  }
};

template <>
struct ScalarEnumerationTraits<mull::Config::DryRunMode> {
  static void enumeration(IO &io, mull::Config::DryRunMode &value) {
    io.enumCase(value, "true",  mull::Config::DryRunMode::Enabled);
    io.enumCase(value, "enabled",  mull::Config::DryRunMode::Enabled);
    io.enumCase(value, "false",  mull::Config::DryRunMode::Disabled);
    io.enumCase(value, "disabled",  mull::Config::DryRunMode::Disabled);
  }
};

template <>
struct ScalarEnumerationTraits<mull::Config::FailFastMode> {
  static void enumeration(IO &io, mull::Config::FailFastMode &value) {
    io.enumCase(value, "enabled",  mull::Config::FailFastMode::Enabled);
    io.enumCase(value, "disabled",  mull::Config::FailFastMode::Disabled);
  }
};

template <>
struct ScalarEnumerationTraits<mull::Config::UseCache> {
  static void enumeration(IO &io, mull::Config::UseCache &value) {
    io.enumCase(value, "true",  mull::Config::UseCache::Yes);
    io.enumCase(value, "yes",  mull::Config::UseCache::Yes);
    io.enumCase(value, "false",  mull::Config::UseCache::No);
    io.enumCase(value, "no",  mull::Config::UseCache::No);
  }
};

template <>
struct ScalarEnumerationTraits<mull::Config::EmitDebugInfo> {
  static void enumeration(IO &io, mull::Config::EmitDebugInfo &value) {
    io.enumCase(value, "true",  mull::Config::EmitDebugInfo::Yes);
    io.enumCase(value, "yes",  mull::Config::EmitDebugInfo::Yes);
    io.enumCase(value, "false",  mull::Config::EmitDebugInfo::No);
    io.enumCase(value, "no",  mull::Config::EmitDebugInfo::No);
  }
};

template <>
struct ScalarEnumerationTraits<mull::Config::Diagnostics> {
  static void enumeration(IO &io, mull::Config::Diagnostics &value) {
    io.enumCase(value, "none",     mull::Config::Diagnostics::None);
    io.enumCase(value, "survived", mull::Config::Diagnostics::Survived);
    io.enumCase(value, "killed",   mull::Config::Diagnostics::Killed);
    io.enumCase(value, "all",      mull::Config::Diagnostics::All);
  }
};

template <>
struct MappingTraits<mull::Config>
{
  static void mapping(IO& io, mull::Config& config)
  {
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
  }
};
}
}

namespace mull {

class ConfigParser {
public:
  Config loadConfig(llvm::yaml::Input &input);
  Config loadConfig(const char *filename);
};
}
