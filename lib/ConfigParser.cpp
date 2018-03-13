#include "ConfigParser.h"

#include "Config.h"
#include "Logger.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>

#include <memory>

using namespace llvm;
using namespace mull;

Config ConfigParser::loadConfig(const char *filename) {
  auto bufferOrError = MemoryBuffer::getFile(filename);
  Config config;

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

  /// FIXME: I could not find a better place for this code.
  if (config.getCustomTests().empty()) {
    config.addCustomTest(CustomTestDefinition("main", "main", "mull", {}));
    config.addCustomTest(CustomTestDefinition("main", "_main", "mull", {}));
  }

  return config;
}

Config ConfigParser::loadConfig(yaml::Input &input) {
  Config config;
  input >> config;

  /// FIXME: I could not find a better place for this code.
  if (config.getCustomTests().empty()) {
    config.addCustomTest(CustomTestDefinition("main", "main", "mull", {}));
    config.addCustomTest(CustomTestDefinition("main", "_main", "mull", {}));
  }
  return config;
}
