#pragma once

struct MullDiagnostics;
struct MullConfig;

namespace llvm {
class Module;
} // namespace llvm

namespace mull {
void mutateBitcode(llvm::Module &module, const MullDiagnostics &diagnostics,
                   const MullConfig &configuration);
} // namespace mull
