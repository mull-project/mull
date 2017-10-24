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
struct MappingTraits<mull::Config>
{
  static void mapping(IO& io, mull::Config& config)
  {
    io.mapOptional("bitcode_file_list", config.bitcodeFileList);
    io.mapOptional("project_name", config.projectName);
    io.mapOptional("test_framework", config.testFramework);
    io.mapOptional("dynamic_library_file_list", config.dynamicLibraryFileList);
    io.mapOptional("mutation_operators", config.mutationOperators);
    io.mapOptional("tests", config.tests);
    io.mapOptional("exclude_locations", config.excludeLocations);
    io.mapOptional("custom_tests", config.customTests);
    io.mapOptional("fork", config.fork);
    io.mapOptional("dry_run", config.dryRun);
    io.mapOptional("use_cache", config.useCache);
    io.mapOptional("emit_debug_info", config.emitDebugInfo);
    io.mapOptional("diagnostics", config.diagnostics);
    io.mapOptional("timeout", config.timeout);
    io.mapOptional("max_distance", config.maxDistance);
    io.mapOptional("cache_directory", config.cacheDirectory);
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
