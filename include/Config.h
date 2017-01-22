#pragma once

#include "Logger.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"

#include "llvm/Support/YAMLTraits.h"

#include <string>
#include <vector>

static int MullDefaultTimeoutMilliseconds = 3000;

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
  std::vector<std::string> mutationOperators;
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
    mutationOperators(
      // Yaml::Traits stops reading mutation_operators from config.yaml
      // if these 3 default operators are set here (BUG?).
      // So leaving out the empty ()
      // {
      //   AddMutationOperator::ID,
      //   NegateConditionMutationOperator::ID,
      //   RemoveVoidFunctionMutationOperator::ID
      // }
    ),
    fork(true),
    dryRun(false),
    useCache(true),
    timeout(MullDefaultTimeoutMilliseconds),
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
         const std::string &cacheDir,
         std::vector<std::string> mutationOperators) :
    bitcodePaths(paths),
    mutationOperators(mutationOperators),
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

  const std::vector<std::string> &getMutationOperators() const {
    return mutationOperators;
  }

  void dump() const {
    Logger::debug() << "Config>\n"
    << "\tdistance: " << getMaxDistance() << '\n'
    << "\tdry_run: " << isDryRun() << '\n'
    << "\tfork: " << getFork() << '\n';

    if (getMutationOperators().size() > 0) {
      Logger::debug() << "\tmutation_operators: " << '\n';
      for (auto mutationOperator : getMutationOperators()) {
        Logger::debug() << "\t- " << mutationOperator << '\n';
      }
    }
  }
};
}

