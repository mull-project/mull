#pragma once

#include <llvm/Object/ObjectFile.h>

#include <string>

namespace mull {

class Bitcode;
class MutationPoint;
class Diagnostics;

class ObjectCache {
  bool useOnDiskCache;
  std::string cacheDirectory;

public:
  ObjectCache(Diagnostics &diagnostics, bool useCache, const std::string &cacheDir);

  llvm::object::OwningBinary<llvm::object::ObjectFile>
  getInstrumentedObject(const Bitcode &bitcode);
  llvm::object::OwningBinary<llvm::object::ObjectFile> getObject(const Bitcode &bitcode);

  void putInstrumentedObject(llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
                             const Bitcode &bitcode);
  void putObject(llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
                 const Bitcode &bitcode);

private:
  llvm::object::OwningBinary<llvm::object::ObjectFile>
  getObjectFromDisk(const std::string &identifier);
  void putObjectOnDisk(llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
                       const std::string &identifier);
};
} // namespace mull
