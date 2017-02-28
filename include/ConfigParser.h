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
struct MappingTraits<mull::Config>
{
  static void mapping(IO& io, mull::Config& config)
  {
    io.mapOptional("project_name", config.projectName);
    io.mapOptional("test_framework", config.testFramework);
    io.mapOptional("bitcode_files", config.bitcodePaths);
    io.mapOptional("mutation_operators", config.mutationOperators);
    io.mapOptional("dynamic_libraries", config.dynamicLibraries);
    io.mapOptional("tests", config.tests);
    io.mapOptional("fork", config.fork);
    io.mapOptional("dry_run", config.dryRun);
    io.mapOptional("use_cache", config.useCache);
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
