#include "Toolchain/ObjectCache.h"

#include "Logger.h"
#include "MullModule.h"
#include "MutationPoint.h"

#include <dirent.h>
#include <sys/stat.h>

using namespace mull;
using namespace llvm;
using namespace llvm::object;

static bool cacheDirectoryExists(const std::string &path) {
  const char *c_path = path.c_str();
  DIR *cacheDir = opendir(c_path);
  if (cacheDir) {
    /// If we can open the dir, then it exists
    closedir(cacheDir);
    return true;
  } else {
    /// Otherwise - attempt to create one
    if (mkdir(c_path, S_IRWXU) == 0) {
      return true;
    }
  }

  return false;
}

ObjectCache::ObjectCache(bool useCache, const std::string &cacheDir)
  : useOnDiskCache(useCache),
    cacheDirectory(cacheDir)
{
  if (useOnDiskCache && !cacheDirectoryExists(cacheDirectory)) {
    Logger::info() << "Cache directory '" << cacheDirectory
                   << "' is not accessible\n";
    Logger::info() << "falling back to in-memory cache\n";
    useOnDiskCache = false;
  }
}

OwningBinary<ObjectFile> ObjectCache::getObjectFromDisk(const std::string &identifier) {
  if (!useOnDiskCache) {
    return OwningBinary<ObjectFile>();
  }

  std::string cacheName(cacheDirectory + "/" + identifier + ".o");

  ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
    MemoryBuffer::getFile(cacheName.c_str());

  if (!buffer) {
    return OwningBinary<ObjectFile>();
  }

  Expected<std::unique_ptr<ObjectFile>> objectOrError =
    ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

  if (!objectOrError) {
    return OwningBinary<ObjectFile>();
  }

  std::unique_ptr<ObjectFile> objectFile(std::move(objectOrError.get()));

  auto owningObject = OwningBinary<ObjectFile>(std::move(objectFile),
                                               std::move(buffer.get()));
  return owningObject;
}

OwningBinary<ObjectFile> ObjectCache::getObject(const MullModule &module) {
  return getObjectFromDisk(module.getUniqueIdentifier());
}

OwningBinary<ObjectFile> ObjectCache::getObject(const MutationPoint &mutationPoint) {
  return getObjectFromDisk(mutationPoint.getUniqueIdentifier());
}

void ObjectCache::putObjectOnDisk(
                  OwningBinary<ObjectFile> &object,
                  const std::string &identifier) {
  if (!useOnDiskCache) {
    return;
  }

  std::string cacheName(cacheDirectory + "/" + identifier + ".o");
  std::error_code EC;
  raw_fd_ostream outfile(cacheName, EC, sys::fs::F_None);
  outfile.write(object.getBinary()->getMemoryBufferRef().getBufferStart(),
                object.getBinary()->getMemoryBufferRef().getBufferSize());
  outfile.close();
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
                            const MullModule &module) {
  putObjectOnDisk(object, module.getUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
                            const MutationPoint &mutationPoint) {
  putObjectOnDisk(object, mutationPoint.getUniqueIdentifier());
}
