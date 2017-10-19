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
  if (useOnDiskCache) {
    Logger::info() << "Unfortunately On-Disk cache is broken at the moment\n";
    Logger::info() << "See this issue for more details: https://github.com/mull-project/mull/issues/234\n";
    Logger::info() << "Falling back to in-memory cache\n";
    useOnDiskCache = false;
  }

  if (useOnDiskCache && !cacheDirectoryExists(cacheDirectory)) {
    Logger::info() << "Cache directory '" << cacheDirectory
                   << "' is not accessible\n";
    Logger::info() << "falling back to in-memory cache\n";
    useOnDiskCache = false;
  }
}

ObjectFile *ObjectCache::getObjectFromMemory(const std::string &identifier) {
  if (inMemoryCache.count(identifier) != 0) {
    auto &owningObject = inMemoryCache.at(identifier);
    return owningObject.getBinary();
  }

  return nullptr;
}

ObjectFile *ObjectCache::getObjectFromDisk(const std::string &identifier) {
  if (!useOnDiskCache) {
    return nullptr;
  }

  std::string cacheName(cacheDirectory + "/" + identifier + ".o");

  ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
    MemoryBuffer::getFile(cacheName.c_str());

  if (!buffer) {
    return nullptr;
  }

  Expected<std::unique_ptr<ObjectFile>> objectOrError =
    ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

  if (!objectOrError) {
    return nullptr;
  }

  std::unique_ptr<ObjectFile> objectFile(std::move(objectOrError.get()));

  auto owningObject = OwningBinary<ObjectFile>(std::move(objectFile),
                                               std::move(buffer.get()));
  auto object = owningObject.getBinary();
  if (object != nullptr) {
    inMemoryCache.insert(std::make_pair(identifier, std::move(owningObject)));
  }

  return object;
}

ObjectFile *ObjectCache::getObject(const std::string &identifier) {
  ObjectFile *objectFile = getObjectFromMemory(identifier);
  if (objectFile == nullptr) {
    objectFile = getObjectFromDisk(identifier);
  }
  return objectFile;
}

ObjectFile *ObjectCache::getObject(const MullModule &module) {
  return getObject(module.getUniqueIdentifier());
}

ObjectFile *ObjectCache::getObject(const MutationPoint &mutationPoint) {
  return getObject(mutationPoint.getUniqueIdentifier());
}

void ObjectCache::putObjectInMemory(
                    llvm::object::OwningBinary<llvm::object::ObjectFile> object,
                    const std::string &identifier) {
  inMemoryCache.insert(std::make_pair(identifier, std::move(object)));
}

void ObjectCache::putObjectOnDisk(
                  llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
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

void ObjectCache::putObject(OwningBinary<ObjectFile> object,
                            const std::string &identifier) {
  putObjectOnDisk(object, identifier);
  putObjectInMemory(std::move(object), identifier);
}

void ObjectCache::putObject(OwningBinary<ObjectFile> object,
                            const MullModule &module) {
  putObject(std::move(object), module.getUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> object,
                            const MutationPoint &mutationPoint) {
  putObject(std::move(object), mutationPoint.getUniqueIdentifier());
}
