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
struct MappingTraits<Mutang::Config>
{
  static void mapping(IO& io, Mutang::Config& config)
  {
    io.mapOptional("bitcode_files", config.bitcodePaths);
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

namespace Mutang {

class ConfigParser {
public:
  Config loadConfig(llvm::yaml::Input &input);
  Config loadConfig(const char *filename);
};
}
