#include <llvm/IR/Module.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>
#include <mull/Driver.h>

namespace {

class MullIRFrontend : public llvm::PassInfoMixin<MullIRFrontend> {
public:
  llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &mam) {
    mull::mutateBitcode(module);
    return llvm::PreservedAnalyses::none();
  }
};

extern "C" __attribute__((visibility("default"))) LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "mull-ir-frontend",
           LLVM_VERSION_STRING,
           [](llvm::PassBuilder &PB) {
             PB.registerPipelineStartEPCallback(
                 [](llvm::ModulePassManager &modulePassManager
#if LLVM_VERSION_MAJOR > 11
                    ,
                    llvm::PassBuilder::OptimizationLevel optimizationLevel
#endif
                 ) { modulePassManager.addPass(MullIRFrontend()); });
           } };
}

} // namespace
