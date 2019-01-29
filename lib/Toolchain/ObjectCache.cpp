#include "Toolchain/ObjectCache.h"

#include "Logger.h"
#include "MullModule.h"
#include "MutationPoint.h"
#include "LLVMCompatibility.h"

using namespace mull;
using namespace llvm::object;

ObjectCache::ObjectCache(bool useCache, const std::string &cacheDir)
  : useOnDiskCache(useCache),
    cacheDirectory(cacheDir)
{
  if (useOnDiskCache) {
    auto error = llvm::sys::fs::create_directories(cacheDir);
    if (error) {
      Logger::info() << "Cache directory '" << cacheDirectory
                     << "' is not accessible\n";
      Logger::info() << "Underlying issue:\n";
      Logger::info() << error.message() << "\n";
      Logger::info() << "falling back to in-memory cache\n";
      useOnDiskCache = false;
    }
  }
}

OwningBinary<ObjectFile> ObjectCache::getObjectFromDisk(const std::string &identifier) {
  if (!useOnDiskCache) {
    return OwningBinary<ObjectFile>();
  }

  std::string cacheName(cacheDirectory + "/" + identifier + ".o");

  auto buffer = llvm::MemoryBuffer::getFile(cacheName.c_str());

  if (!buffer) {
    return OwningBinary<ObjectFile>();
  }

  llvm_compat::Expected<std::unique_ptr<ObjectFile>> objectOrError =
    ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

  if (!objectOrError) {
    return OwningBinary<ObjectFile>();
  }

  std::unique_ptr<ObjectFile> objectFile(std::move(objectOrError.get()));

  auto owningObject = OwningBinary<ObjectFile>(std::move(objectFile),
                                               std::move(buffer.get()));
  return owningObject;
}

OwningBinary<ObjectFile> ObjectCache::getInstrumentedObject(const MullModule &module) {
  return getObjectFromDisk(module.getInstrumentedUniqueIdentifier());
}

OwningBinary<ObjectFile> ObjectCache::getObject(const MullModule &module) {
  return getObjectFromDisk(module.getMutatedUniqueIdentifier());
}

void ObjectCache::putObjectOnDisk(OwningBinary<ObjectFile> &object,
                                  const std::string &identifier) {
  if (!useOnDiskCache) {
    return;
  }

  std::string cacheName(cacheDirectory + "/" + identifier + ".o");
  std::error_code EC;
  llvm::raw_fd_ostream outfile(cacheName, EC, llvm::sys::fs::F_None);
  outfile.write(object.getBinary()->getMemoryBufferRef().getBufferStart(),
                object.getBinary()->getMemoryBufferRef().getBufferSize());
  outfile.close();
}

void ObjectCache::putInstrumentedObject(OwningBinary<ObjectFile> &object,
                                        const MullModule &module) {
  putObjectOnDisk(object, module.getInstrumentedUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
                            const MullModule &module) {
  putObjectOnDisk(object, module.getMutatedUniqueIdentifier());
}
