#include <llvm/IR/Module.h>
#include <llvm/Passes/PassBuilder.h>
#if LLVM_VERSION_MAJOR > 21
#include <llvm/Plugins/PassPlugin.h>
#else
#include <llvm/Passes/PassPlugin.h>
#endif
#include "rust/mull-cxx-bridge/bridge.rs.h"
#include <llvm/Support/raw_ostream.h>
#include <mull/Driver.h>

namespace {

// Markers read by `mull-runner --version`; `retain` keeps them under --gc-sections
#if defined(__APPLE__)
#define MULL_KEEP __attribute__((used))
#else
#define MULL_KEEP __attribute__((used, retain))
#endif
MULL_KEEP const char llvmVersionMarker[] = "mull-plugin-llvm:" LLVM_VERSION_STRING;
MULL_KEEP rust::Str (*const mullVersionMarker)() = plugin_version_marker;

class MullIRFrontend : public llvm::PassInfoMixin<MullIRFrontend> {
public:
  llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &mam) {
    auto core = init_core_ffi(DiagOutput::Stdout);
    mull::mutateBitcode(module, core->diag(), core->config());
    return llvm::PreservedAnalyses::none();
  }
};

extern "C" __attribute__((visibility("default"))) LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  using OptimizationLevel = llvm::OptimizationLevel;

  return { LLVM_PLUGIN_API_VERSION,
           "mull-ir-frontend",
           LLVM_VERSION_STRING,
           [](llvm::PassBuilder &PB) {
             PB.registerPipelineStartEPCallback([](llvm::ModulePassManager &modulePassManager,
                                                   OptimizationLevel optimizationLevel) {
               modulePassManager.addPass(MullIRFrontend());
             });
           } };
}

} // namespace
