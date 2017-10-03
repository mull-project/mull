#pragma once

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IndirectionUtils.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/LogicalDylib.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Debug.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <llvm/ExecutionEngine/Orc/OrcABISupport.h>

#include <stdlib.h>

#include <queue>
#include <list>
#include <memory>
#include <set>
#include <stack>
#include <sys/mman.h>
#include <sys/types.h>
#include <utility>

#include "DynamicCallTree.h"

using namespace llvm;

namespace mull {

class MutationPoint;

extern uint64_t *_callTreeMapping;
extern std::stack<uint64_t> _callstack;

extern "C" void mull_enterFunction(uint64_t functionIndex);
extern "C" void mull_leaveFunction(uint64_t functionIndex);

/// ORC's implementation deletes body of original function
/// We need to preserve it!
/// TODO: Ask for the reason of such behaviour
void mull_moveFunctionBody(Function &OrigF, ValueToValueMapTy &VMap,
                           ValueMaterializer *Materializer,
                           Function *NewF = nullptr);

typedef llvm::orc::ObjectLinkingLayer<> LinkingLayer;

class MullJIT {
private:
  llvm::orc::SimpleCompiler compiler;
  LinkingLayer linkingLayer;

  template <typename MaterializerFtor>
  class LambdaMaterializer final : public llvm::ValueMaterializer {
  public:
    LambdaMaterializer(MaterializerFtor M) : M(std::move(M)) {}
    llvm::Value *materialize(llvm::Value *V) final { return M(V); }

  private:
    MaterializerFtor M;
  };

  template <typename MaterializerFtor>
  LambdaMaterializer<MaterializerFtor>
  createLambdaMaterializer(MaterializerFtor M) {
    return LambdaMaterializer<MaterializerFtor>(std::move(M));
  }

  struct LogicalModuleResources {
    llvm::Module *SourceModule;
    std::set<const llvm::Function *> StubsToClone;
    std::unique_ptr<llvm::orc::IndirectStubsManager> StubsMgr;

    LogicalModuleResources() = default;

    llvm::orc::JITSymbol findSymbol(llvm::StringRef Name, bool ExportedSymbolsOnly) {
      if (Name.endswith("$stub_ptr") && !ExportedSymbolsOnly) {
        assert(!ExportedSymbolsOnly && "Stubs are never exported");
        return StubsMgr->findPointer(Name.drop_back(9));
      }
      return StubsMgr->findStub(Name, ExportedSymbolsOnly);
    }
  };

  struct LogicalModule {
    // Make this move-only to ensure they don't get duplicated across moves of
    // LogicalDylib or anything like that.
    LogicalModule(LogicalModule &&RHS)
    : Resources(std::move(RHS.Resources)),
    BaseLayerHandles(std::move(RHS.BaseLayerHandles)) {}

    LogicalModule() = default;
    LogicalModuleResources Resources;
    std::vector<LinkingLayer::ObjSetHandleT> BaseLayerHandles;
  };

  std::list<LogicalModule> logicalModules;
  std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memoryManager;
  std::unique_ptr<llvm::RuntimeDyld::SymbolResolver> symbolResolver;
public:

  /// @brief Construct a compile-on-demand layer instance.
  MullJIT(llvm::TargetMachine &machine);
  ~MullJIT();

  void addModuleSet(std::vector<llvm::Module *> &modules,
                    std::unique_ptr<SectionMemoryManager> memoryManager,
                    std::unique_ptr<RuntimeDyld::SymbolResolver> resolver);

  /// @brief Search for the given named symbol.
  /// @param Name The name of the symbol to search for.
  /// @param ExportedSymbolsOnly If true, search only for exported symbols.
  /// @return A handle for the given named symbol, if it exists.
  llvm::orc::JITSymbol findSymbol(StringRef Name, bool ExportedSymbolsOnly);

#pragma mark - Call Tree Begin

  DynamicCallTree *getDynamicCallTree() { return &dynamicCallTree; }
  void prepareForExecution();
  std::unique_ptr<CallTree> createCallTree();
  void cleanupCallTree(std::unique_ptr<CallTree> root);

#pragma mark - Call Tree End

  void setCurrentMutationPoint(MutationPoint *point) {
    this->currentMutationPoint = point;
  }

private:
  void addLogicalModule(llvm::Module *module);

  static std::string mangle(llvm::StringRef Name, const llvm::DataLayout &DL);

  llvm::orc::TargetAddress extractAndCompile(std::list<LogicalModule>::iterator LMH,
                                             Function &F,
                                             uint64_t functionIndex);

  LinkingLayer::ObjSetHandleT emitFunction(std::list<LogicalModule>::iterator LMH,
                                           Function &function,
                                           uint64_t functionIndex);
  void tryToApplyMutation(Module *module, Function *originalFunction, Function *function);

  void insertCallTreeCallbacks(Module *module, Function *originalFunction, Function &function, uint64_t index);

  std::unique_ptr<llvm::orc::JITCompileCallbackManager> callbackManager;
  std::function<std::unique_ptr<llvm::orc::IndirectStubsManager>()> stubsManagerBuilder;

  std::vector<CallTreeFunction> functions;
  DynamicCallTree dynamicCallTree;
  MutationPoint *currentMutationPoint;

  llvm::orc::JITSymbol findSymbolInLogicalModule(std::list<LogicalModule>::iterator LMH,
                                                 const std::string &Name,
                                                 bool ExportedSymbolsOnly) {
    if (auto StubSym = LMH->Resources.findSymbol(Name, ExportedSymbolsOnly))
      return StubSym;

    for (auto BLH : LMH->BaseLayerHandles)
      if (auto Symbol = linkingLayer.findSymbolIn(BLH, Name, ExportedSymbolsOnly))
        return Symbol;
    return nullptr;
  }

  llvm::orc::JITSymbol findSymbolInternally(std::list<LogicalModule>::iterator LMH,
                                            const std::string &Name) {
    if (auto Symbol = findSymbolInLogicalModule(LMH, Name, false))
      return Symbol;

    for (auto LMI = logicalModules.begin(), LME = logicalModules.end(); LMI != LME; ++LMI) {
      if (LMI != LMH)
        if (auto Symbol = findSymbolInLogicalModule(LMI, Name, false))
          return Symbol;
    }

    return nullptr;
  }
};

}
