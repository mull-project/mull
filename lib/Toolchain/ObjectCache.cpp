#include "mull/Toolchain/ObjectCache.h"

#include "LLVMCompatibility.h"
#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/MutationPoint.h"
#include <sstream>

using namespace mull;
using namespace llvm::object;

ObjectCache::ObjectCache(Diagnostics &diagnostics, bool useCache, const std::string &cacheDir)
    : useOnDiskCache(useCache), cacheDirectory(cacheDir) {
  if (useOnDiskCache) {
    auto error = llvm::sys::fs::create_directories(cacheDir);
    if (error) {
      std::stringstream stringstream;
      stringstream << "Cache directory '" << cacheDirectory << "' is not accessible\n"
                   << "Underlying issue:\n"
                   << error.message() << "\n"
                   << "falling back to in-memory cache";
      diagnostics.warning(stringstream.str());
      useOnDiskCache = false;
    }
  }
}

OwningBinary<ObjectFile> ObjectCache::getObjectFromDisk(const std::string &identifier) {
  if (!useOnDiskCache) {
    return OwningBinary<ObjectFile>();
  }

  std::string cacheName(cacheDirectory + "/" + identifier + ".o");

  auto buffer = llvm::MemoryBuffer::getFile(cacheName);

  if (!buffer) {
    return OwningBinary<ObjectFile>();
  }

  llvm_compat::Expected<std::unique_ptr<ObjectFile>> objectOrError =
      ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

  if (!objectOrError) {
    return OwningBinary<ObjectFile>();
  }

  std::unique_ptr<ObjectFile> objectFile(std::move(objectOrError.get()));

  auto owningObject = OwningBinary<ObjectFile>(std::move(objectFile), std::move(buffer.get()));
  return owningObject;
}

OwningBinary<ObjectFile> ObjectCache::getInstrumentedObject(const Bitcode &bitcode) {
  return getObjectFromDisk(bitcode.getInstrumentedUniqueIdentifier());
}

OwningBinary<ObjectFile> ObjectCache::getObject(const Bitcode &bitcode) {
  return getObjectFromDisk(bitcode.getMutatedUniqueIdentifier());
}

void ObjectCache::putObjectOnDisk(OwningBinary<ObjectFile> &object, const std::string &identifier) {
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

void ObjectCache::putInstrumentedObject(OwningBinary<ObjectFile> &object, const Bitcode &bitcode) {
  putObjectOnDisk(object, bitcode.getInstrumentedUniqueIdentifier());
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object, const Bitcode &bitcode) {
  putObjectOnDisk(object, bitcode.getMutatedUniqueIdentifier());
}
