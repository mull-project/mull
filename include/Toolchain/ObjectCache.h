#pragma once

#include "llvm/Object/ObjectFile.h"

namespace Mutang {
  class MutangModule;
  class MutationPoint;

  class ObjectCache {
  public:
    ObjectCache();

    llvm::object::OwningBinary<llvm::object::ObjectFile>
                                      getObject(const MutangModule &module);
    llvm::object::OwningBinary<llvm::object::ObjectFile>
                                  getObject(const MutationPoint &mutationPoint);

    void putObject(llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
                   const MutangModule &module);

    void putObject(llvm::object::OwningBinary<llvm::object::ObjectFile> &object,
                   const MutationPoint &mutationPoint);
  };
}
