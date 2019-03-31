#include "mull/ObjectLoader.h"

#include "mull/Config/Configuration.h"
#include "mull/Logger.h"

using namespace mull;

ObjectFiles ObjectLoader::loadObjectFiles(Configuration &configuration) {
  ObjectFiles objectFiles;

  for (auto &path : configuration.objectFilePaths) {
    auto objectFile = llvm::object::ObjectFile::createObjectFile(path);
    if (!objectFile) {
      Logger::error() << "Cannot load object file: " << path << "\n";
    }
    objectFiles.emplace_back(std::move(objectFile.get()));
  }

  return std::move(objectFiles);
}
