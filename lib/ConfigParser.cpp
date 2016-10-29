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
  float timeout = MutangDefaultTimeout;
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

    else if (key->getRawValue().equals(StringRef("timeout"))) {
      auto value = dyn_cast<yaml::ScalarNode>(keyValue.getValue());

      std::string timeoutStringValue = value->getRawValue().str();

      double timeoutValue = ::atof(timeoutStringValue.c_str());

      assert(timeoutValue > 0 &&
             "ConfigParser error: timeout value was provided but we could not resolve it to a meaningful value.");

      timeout = timeoutValue;
    }
  }

  return make_unique<Config>(paths, fork, timeout);
}
