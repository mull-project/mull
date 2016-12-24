#include "ConfigParser.h"

#include "Config.h"
#include "Logger.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include <memory>

using namespace llvm;
using namespace mull;

Config ConfigParser::loadConfig(const char *filename) {
  auto bufferOrError = MemoryBuffer::getFile(filename);

  if (!bufferOrError) {
    Logger::error() << "Can't read config file: " << filename << '\n';
  }

  auto buffer = bufferOrError->get();
  llvm::yaml::Input yin(buffer->getBuffer());

  Config config;
  yin >> config;

  if (yin.error()) {
    Logger::error() << "Failed to parse YAML file: " << filename << '\n';
  }

  return config;
}

Config ConfigParser::loadConfig(yaml::Input &input) {
  Config config;
  input >> config;
  return config;
}
