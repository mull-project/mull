#include "Toolchain/ObjectCache.h"

using namespace Mutang;
using namespace llvm;
using namespace llvm::object;

ObjectCache::ObjectCache() {}

OwningBinary<ObjectFile> ObjectCache::getObject(const MutangModule &module) {
  return OwningBinary<ObjectFile>();
}

OwningBinary<ObjectFile> ObjectCache::getObject(const MutationPoint &mutationPoint) {
  return OwningBinary<ObjectFile>();
}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
               const MutangModule &module) {

}

void ObjectCache::putObject(OwningBinary<ObjectFile> &object,
                            const MutationPoint &mutationPoint) {

}
