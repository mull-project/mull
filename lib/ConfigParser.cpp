#include "Config.h"
#include "ConfigParser.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include <memory>

using namespace llvm;
using namespace Mutang;

Config ConfigParser::loadConfig(const char *filename) {
  auto bufferOrError = MemoryBuffer::getFile(filename);

  if (!bufferOrError) {
    printf("Can't read config file '%s'\n", filename);
  }

  auto buffer = bufferOrError->get();
  llvm::yaml::Input yin(buffer->getBuffer());

  Config config;
  yin >> config;

  if (yin.error()) {
    printf("Failed to parse YAML file: '%s'", filename);
  }

  return config;
}

Config ConfigParser::loadConfig(yaml::Input &input) {
  Config config;
  input >> config;
  return config;
}
