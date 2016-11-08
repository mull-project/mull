#include "Toolchain/ObjectCache.h"
#include "MutangModule.h"
#include "MutationPoint.h"

using namespace Mutang;
using namespace llvm;
using namespace llvm::object;

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

  Expected<std::unique_ptr<ObjectFile>> object =
  ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

  if (!object) {
    return OwningBinary<ObjectFile>();
  }

  printf("Cache hit for '%s'\n", uniqueID.c_str());

  return OwningBinary<ObjectFile>(std::move(*object),
                                  std::move(buffer.get()));
}

ObjectCache::ObjectCache() {}

OwningBinary<ObjectFile> ObjectCache::getObject(const MutangModule &module) {
  return loadFileFromCache(module.getUniqueIdentifier());
}

OwningBinary<ObjectFile> ObjectCache::getObject(const MutationPoint &mutationPoint) {
  return loadFileFromCache(mutationPoint.getUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
                            const MutangModule &module) {
  putFileIntoCache(object.getBinary(), module.getUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
                            const MutationPoint &mutationPoint) {
  putFileIntoCache(object.getBinary(), mutationPoint.getUniqueIdentifier());
}
