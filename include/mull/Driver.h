#pragma once

namespace llvm {
class Module;
} // namespace llvm

namespace mull {
void mutateBitcode(llvm::Module &module);
} // namespace mull
