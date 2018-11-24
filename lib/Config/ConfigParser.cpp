#include "Config/ConfigParser.h"

#include "Config/RawConfig.h"
#include "Logger.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>

#include <memory>

using namespace llvm;
using namespace mull;

static void addDefaultCustomTests(RawConfig &config) {
  /// FIXME: I could not find a better place for this code.
  if (config.getCustomTests().empty()) {
    config.addCustomTest(CustomTestDefinition("main", "main", "mull", {}));
    config.addCustomTest(CustomTestDefinition("main", "_main", "mull", {}));
  }
}

static void fixupConfig(RawConfig &config) {
  addDefaultCustomTests(config);
  config.normalizeParallelizationConfig();
}

RawConfig ConfigParser::loadConfig(const char *filename) {
  auto bufferOrError = MemoryBuffer::getFile(filename);
  RawConfig config;

  if (!bufferOrError) {
    Logger::error() << "Can't read config file: " << filename << '\n';
  } else {
    auto buffer = bufferOrError->get();
    llvm::yaml::Input yin(buffer->getBuffer());

    yin >> config;

    if (yin.error()) {
      Logger::error() << "Failed to parse YAML file: " << filename << '\n';
    }
  }

  fixupConfig(config);

  return config;
}

RawConfig ConfigParser::loadConfig(yaml::Input &input) {
  RawConfig config;
  input >> config;
  fixupConfig(config);
  return config;
}
