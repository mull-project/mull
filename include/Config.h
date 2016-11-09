#pragma once

#include <string>
#include <vector>

static int MutangDefaultTimeout = 3000;

namespace Mutang {

class Config {
  std::vector<std::string> bitcodePaths;
  bool fork;
  bool dryRun;
  bool useCache;
  int timeout;
  int maxDistance;
  std::string cacheDirectory;

public:
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
