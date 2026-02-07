#pragma once

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolicFile.h>
#include <memory>
struct MullDiagnostics;
namespace mull {
typedef std::pair<std::unique_ptr<llvm::MemoryBuffer>, std::unique_ptr<llvm::object::ObjectFile>>
    OwnedObjectFile;
OwnedObjectFile loadObjectFile(const MullDiagnostics &diagnostics,
                               const std::string &executablePath);
} // namespace mull
