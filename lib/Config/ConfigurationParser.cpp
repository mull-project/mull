#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/YAMLTraits.h>

using namespace mull;
using namespace std::string_literals;

#include "GeneratedYamlMapping.cpp"

std::string Configuration::findConfig(Diagnostics &diagnostics) {
  if (getenv("MULL_CONFIG") != nullptr) {
    return getenv("MULL_CONFIG");
  }
  llvm::SmallString<PATH_MAX> cwd;
  std::error_code ec = llvm::sys::fs::current_path(cwd);
  if (ec) {
    diagnostics.warning("Cannot find config: "s + ec.message());
    return {};
  }
  std::string currentDir = cwd.str().str();
  auto config = currentDir + "/mull.yml";
  if (llvm::sys::fs::exists(config)) {
    return config;
  }
  while (llvm::sys::path::has_parent_path(currentDir)) {
    currentDir = llvm::sys::path::parent_path(currentDir);
    config = currentDir + "/mull.yml";
    if (llvm::sys::fs::exists(config)) {
      return config;
    }
  }
  return {};
}

Configuration Configuration::loadFromDisk(Diagnostics &diagnostics, const std::string &path) {
  auto bufferOrError = llvm::MemoryBuffer::getFile(path);
  if (!bufferOrError) {
    diagnostics.warning("Cannot read config: "s + bufferOrError.getError().message());
    return {};
  }
  llvm::yaml::Input input(bufferOrError.get()->getMemBufferRef());
  Configuration configuration;
  input >> configuration;
  configuration.pathOnDisk = path;
  return configuration;
}
