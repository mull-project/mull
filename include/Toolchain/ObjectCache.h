#pragma once

#include "llvm/Object/ObjectFile.h"

#include <string>
#include <map>

namespace mull {
  class MullModule;
  class IMutationPoint;

  class ObjectCache {
    std::map<std::string, llvm::object::OwningBinary<llvm::object::ObjectFile>> inMemoryCache;
    bool useOnDiskCache;
    std::string cacheDirectory;

  public:
    ObjectCache(bool useCache, const std::string &cacheDir);

    llvm::object::ObjectFile *getObject(const MullModule &module);
    llvm::object::ObjectFile *getObject(const IMutationPoint &mutationPoint);

    void putObject(llvm::object::OwningBinary<llvm::object::ObjectFile> object,
                   const MullModule &module);

    void putObject(llvm::object::OwningBinary<llvm::object::ObjectFile> object,
                   const IMutationPoint &mutationPoint);

  private:
    llvm::object::ObjectFile *getObject(const std::string &identifier);
    void putObject(llvm::object::OwningBinary<llvm::object::ObjectFile> object,
                   const std::string &identifier);

    llvm::object::ObjectFile *getObjectFromMemory(const std::string &identifier);
    llvm::object::ObjectFile *getObjectFromDisk(const std::string &identifier);

    void putObjectInMemory(llvm::object::OwningBinary<llvm::object::ObjectFile> object,
                           const std::string &identifier);
    void putObjectOnDisk(llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
                         const std::string &identifier);
  };
}
