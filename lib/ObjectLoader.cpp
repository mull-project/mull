#include "mull/ObjectLoader.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

using namespace mull;

ObjectFiles ObjectLoader::loadObjectFiles(Configuration &configuration, Diagnostics &diagnostics) {
  ObjectFiles objectFiles;

  for (auto &path : configuration.objectFilePaths) {
    auto objectFile = llvm::object::ObjectFile::createObjectFile(path);
    if (!objectFile) {
      diagnostics.warning(std::string("Cannot load object file: ") + path);
    }
    objectFiles.emplace_back(std::move(objectFile.get()));
  }

  return std::move(objectFiles);
}
