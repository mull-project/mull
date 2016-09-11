#include "ConfigParser.h"
#include "Config.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include <memory>

using namespace llvm;
using namespace Mutang;

std::unique_ptr<Config> ConfigParser::loadConfig(const char *filename) {
  SourceMgr SM;

  auto BufOrErr = MemoryBuffer::getFile(filename);

  if (!BufOrErr) {
    printf("can't read config file '%s'\n", filename);
    exit(1);
  }

  auto Buffer = BufOrErr->get();

  yaml::Stream YAMLBuf(Buffer->getMemBufferRef(), SM);

  return loadConfig(YAMLBuf);
}

std::unique_ptr<Config> ConfigParser::loadConfig(yaml::Stream &S) {
  /// Fork is enabled by default
  bool fork = true;

  auto Paths = std::vector<std::string>();

  yaml::Node *RootNode = S.begin()->getRoot();
  assert(RootNode && "Expected config file to be a valid yaml document");

  auto MappingNode = dyn_cast<yaml::MappingNode>(RootNode);

  for (auto &KeyValue : *MappingNode) {
    auto Key = dyn_cast<yaml::ScalarNode>(KeyValue.getKey());

    if (Key->getRawValue().equals(StringRef("bitcode_files"))) {
      auto Values = dyn_cast<yaml::SequenceNode>(KeyValue.getValue());

      for (auto &V : *Values) {
        auto BitcodePath = dyn_cast<yaml::ScalarNode>(&V);
        assert(BitcodePath && "bitcode_files should contain strings (scalars)");
        Paths.push_back(BitcodePath->getRawValue().str());
      }
    }

    else if (Key->getRawValue().equals(StringRef("fork"))) {
      auto Value = dyn_cast<yaml::ScalarNode>(KeyValue.getValue());

      if (Value->getRawValue().equals(StringRef("true"))) {
        fork = true;
      } else {
        fork = false;
      }
    }
  }

  return make_unique<Config>(Paths, fork);
}
