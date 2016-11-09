#include "Toolchain/ObjectCache.h"
#include "MutangModule.h"
#include "MutationPoint.h"

using namespace Mutang;
using namespace llvm;
using namespace llvm::object;

static bool cacheDirectoryExists(const std::string &cacheDirectory) {

  return true;
}

static void putFileIntoCache(ObjectFile *object, std::string uniqueID) {
  std::string cacheName("/tmp/mutang_cache/" + uniqueID + ".o");
  std::error_code EC;
  raw_fd_ostream outfile(cacheName, EC, sys::fs::F_None);
  outfile.write(object->getMemoryBufferRef().getBufferStart(),
                object->getMemoryBufferRef().getBufferSize());
  outfile.close();
}

static OwningBinary<ObjectFile> loadFileFromCache(std::string uniqueID) {
  std::string cacheName("/tmp/mutang_cache/" + uniqueID + ".o");

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

  return OwningBinary<ObjectFile>(std::move(objectFile),
                                  std::move(buffer.get()));
}

ObjectCache::ObjectCache() {}

ObjectFile *ObjectCache::getObject(const std::string &identifier) {
  ObjectFile *objectFile = nullptr;

  if (inMemoryCache.count(identifier) != 0) {
    auto &owningObject = inMemoryCache.at(identifier);
    objectFile = owningObject.getBinary();
  } else {
    auto owningObject = loadFileFromCache(identifier);
    objectFile = owningObject.getBinary();
    if (objectFile != nullptr) {
      inMemoryCache.insert(std::make_pair(identifier, std::move(owningObject)));
    }
  }

  if (objectFile) {
    printf("Cache hit for '%s'\n", identifier.c_str());
  }

  return objectFile;
}

void ObjectCache::putObject(OwningBinary<ObjectFile> object,
                            const std::string &identifier) {
  putFileIntoCache(object.getBinary(), identifier);
  inMemoryCache.insert(std::make_pair(identifier, std::move(object)));
}

ObjectFile *ObjectCache::getObject(const MutangModule &module) {
  return getObject(module.getUniqueIdentifier());
}

ObjectFile *ObjectCache::getObject(const MutationPoint &mutationPoint) {
  return getObject(mutationPoint.getUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> object,
                            const MutangModule &module) {
  putObject(std::move(object), module.getUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> object,
                            const MutationPoint &mutationPoint) {
  putObject(std::move(object), mutationPoint.getUniqueIdentifier());
}
