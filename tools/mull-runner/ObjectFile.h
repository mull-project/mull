#pragma once

#include "mull/Diagnostics/Diagnostics.h"
#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolicFile.h>
#include <memory>

namespace mull {
typedef std::pair<std::unique_ptr<llvm::MemoryBuffer>, std::unique_ptr<llvm::object::ObjectFile>>
    OwnedObjectFile;
OwnedObjectFile loadObjectFile(mull::Diagnostics &diagnostics, const std::string &executablePath);
} // namespace mull
