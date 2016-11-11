#include "ConfigParser.h"
#include "Config.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include <memory>

using namespace llvm;
using namespace Mutang;

std::unique_ptr<Config> ConfigParser::loadConfig(const char *filename) {
  SourceMgr sourceManager;

  auto bufferOrError = MemoryBuffer::getFile(filename);

  if (!bufferOrError) {
    printf("can't read config file '%s'\n", filename);
    exit(1);
  }

  auto buffer = bufferOrError->get();

  yaml::Stream yamlBuffer(buffer->getMemBufferRef(), sourceManager);

  return loadConfig(yamlBuffer);
}

std::unique_ptr<Config> ConfigParser::loadConfig(yaml::Stream &stream) {
  /// Fork is enabled by default
  bool fork = true;
  bool dryRun = false;
  bool useCache = true;
  int timeout = MutangDefaultTimeout;
  int maxDistance = 128;
  std::string cacheDirectory = "/tmp/mutang_cache";
  auto paths = std::vector<std::string>();

  yaml::Node *rootNode = stream.begin()->getRoot();
  assert(rootNode && "Expected config file to be a valid yaml document");

  auto mappingNode = dyn_cast<yaml::MappingNode>(rootNode);

  for (auto &keyValue : *mappingNode) {
    auto key = dyn_cast<yaml::ScalarNode>(keyValue.getKey());

    if (key->getRawValue().equals(StringRef("bitcode_files"))) {
      auto values = dyn_cast<yaml::SequenceNode>(keyValue.getValue());

      for (auto &value : *values) {
        auto bitcodePath = dyn_cast<yaml::ScalarNode>(&value);
        assert(bitcodePath && "bitcode_files should contain strings (scalars)");
        paths.push_back(bitcodePath->getRawValue().str());
      }
    }

    else if (key->getRawValue().equals(StringRef("fork"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      if (value->getRawValue().equals(StringRef("true"))) {
        fork = true;
      } else {
        fork = false;
      }
    }

    else if (key->getRawValue().equals(StringRef("dryRun"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      if (value->getRawValue().equals(StringRef("true"))) {
        dryRun = true;
      } else {
        dryRun = false;
      }
    }

    else if (key->getRawValue().equals(StringRef("use_cache"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      if (value->getRawValue().equals(StringRef("true"))) {
        useCache = true;
      } else {
        useCache = false;
      }
    }

    else if (key->getRawValue().equals(StringRef("timeout"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      std::string timeoutStringValue = value->getRawValue().str();

      int timeoutValue = ::atoi(timeoutStringValue.c_str());

      assert(timeoutValue > 0 &&
             "ConfigParser error: timeout value was provided but we could not resolve it to a meaningful value.");

      timeout = timeoutValue;
    }

    else if (key->getRawValue().equals(StringRef("maxDistance"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      std::string timeoutStringValue = value->getRawValue().str();

      int distance = ::atoi(timeoutStringValue.c_str());

      assert(distance > 0 &&
             "ConfigParser error: distance should be more than zero.");

      maxDistance = distance;
    }

    else if (key->getRawValue().equals(StringRef("cache_directory"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      cacheDirectory = value->getRawValue().str();
    }
  }

  return make_unique<Config>(paths, fork, dryRun, useCache, timeout,
                             maxDistance, cacheDirectory);
}
