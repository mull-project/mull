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
    auto Paths = std::vector<std::string>();

    auto Cfg = dyn_cast<yaml::MappingNode>(S.begin()->getRoot());
    assert(Cfg && "Expected to see an object as a root");

    for (auto KeyValue : *Cfg) {
        auto Key = dyn_cast<yaml::ScalarNode>(KeyValue.getKey());
        assert(Key && "keys should be strings (scalars)");
        if (Key->getRawValue().equals(StringRef("bitcode_files"))) {
            auto Values = dyn_cast<yaml::SequenceNode>(KeyValue.getValue());

            for (auto &V : *Values) {
                auto BitcodePath = dyn_cast<yaml::ScalarNode>(&V);
                assert(BitcodePath && "bitcode_files should contain strings (scalars)");
                Paths.push_back(BitcodePath->getRawValue().str());
            }
        }
    }

    return make_unique<Config>(Paths);
}
