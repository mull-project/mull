#pragma once

#include "llvm/Support/YAMLTraits.h"

#include <string>
#include <vector>

static int MutangDefaultTimeout = 3000;

// We need these forward declarations to make our config friends with the
// mapping traits.
namespace mull {
class Config;
}
namespace llvm {
namespace yaml {
template<typename T>
struct MappingTraits;
}
}
namespace mull {

class Config {
  std::vector<std::string> bitcodePaths;
  bool fork;
  bool dryRun;
  bool useCache;
  int timeout;
  int maxDistance;
  std::string cacheDirectory;

  friend llvm::yaml::MappingTraits<mull::Config>;
public:
  // Constructor initializes defaults.
  // TODO: Refactoring into constants.
  Config() :
    bitcodePaths(),
    fork(true),
    dryRun(false),
    useCache(true),
    timeout(MutangDefaultTimeout),
    maxDistance(128),
    cacheDirectory("/tmp/mull_cache")
  {
  }

  Config(const std::vector<std::string> &paths,
         bool fork,
         bool dryrun,
         bool cache,
         int timeout,
         int distance,
         const std::string &cacheDir) :
    bitcodePaths(paths),
    fork(fork),
    dryRun(dryrun),
    useCache(cache),
    timeout(timeout),
    maxDistance(distance),
    cacheDirectory(cacheDir)
  {
  }

  const std::vector<std::string> &getBitcodePaths() const {
      return bitcodePaths;
  }

  bool getFork() const {
    return fork;
  }

  int getTimeout() const {
    return timeout;
  }

  bool getUseCache() const {
    return useCache;
  }

  bool isDryRun() const {
    return dryRun;
  }

  int getMaxDistance() const {
    return maxDistance;
  }

  std::string getCacheDirectory() const {
    return cacheDirectory;
  }

};
}

