#pragma once

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IndirectionUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
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
typedef llvm::orc::IRCompileLayer<LinkingLayer> CompileLayer;

template <typename LogicalModuleResources,
typename LogicalDylibResources>
class MullLogicalDylib {
public:
  typedef typename CompileLayer::ModuleSetHandleT BaseLayerModuleSetHandleT;
private:

  typedef std::vector<BaseLayerModuleSetHandleT> BaseLayerHandleList;

  struct LogicalModule {
    // Make this move-only to ensure they don't get duplicated across moves of
    // LogicalDylib or anything like that.
    LogicalModule(LogicalModule &&RHS)
    : Resources(std::move(RHS.Resources)),
    BaseLayerHandles(std::move(RHS.BaseLayerHandles)) {}

    LogicalModule() = default;
    LogicalModuleResources Resources;
    BaseLayerHandleList BaseLayerHandles;
  };
  typedef std::list<LogicalModule> LogicalModuleList;

public:

  typedef typename BaseLayerHandleList::iterator BaseLayerHandleIterator;
  typedef typename LogicalModuleList::iterator LogicalModuleHandle;

  MullLogicalDylib(CompileLayer &BaseLayer) : BaseLayer(BaseLayer) {}

  ~MullLogicalDylib() {
    for (auto &LM : LogicalModules)
      for (auto BLH : LM.BaseLayerHandles)
        BaseLayer.removeModuleSet(BLH);
  }

  // If possible, remove this and ~LogicalDylib once the work in the dtor is
  // moved to members (eg: self-unregistering base layer handles).
  MullLogicalDylib(MullLogicalDylib &&RHS)
  : BaseLayer(std::move(RHS.BaseLayer)),
    LogicalModules(std::move(RHS.LogicalModules)),
    DylibResources(std::move(RHS.DylibResources)) {}

  LogicalModuleHandle createLogicalModule() {
    LogicalModules.push_back(LogicalModule());
    return std::prev(LogicalModules.end());
  }

  void addToLogicalModule(LogicalModuleHandle LMH,
                          BaseLayerModuleSetHandleT BaseLayerHandle) {
    LMH->BaseLayerHandles.push_back(BaseLayerHandle);
  }

  LogicalModuleResources& getLogicalModuleResources(LogicalModuleHandle LMH) {
    return LMH->Resources;
  }

  BaseLayerHandleIterator moduleHandlesBegin(LogicalModuleHandle LMH) {
    return LMH->BaseLayerHandles.begin();
  }

  BaseLayerHandleIterator moduleHandlesEnd(LogicalModuleHandle LMH) {
    return LMH->BaseLayerHandles.end();
  }

  llvm::orc::JITSymbol findSymbolInLogicalModule(LogicalModuleHandle LMH,
                                      const std::string &Name,
                                      bool ExportedSymbolsOnly) {

    if (auto StubSym = LMH->Resources.findSymbol(Name, ExportedSymbolsOnly))
      return StubSym;

    for (auto BLH : LMH->BaseLayerHandles)
      if (auto Symbol = BaseLayer.findSymbolIn(BLH, Name, ExportedSymbolsOnly))
        return Symbol;
    return nullptr;
  }

  llvm::orc::JITSymbol findSymbolInternally(LogicalModuleHandle LMH,
                                 const std::string &Name) {
    if (auto Symbol = findSymbolInLogicalModule(LMH, Name, false))
      return Symbol;

    for (auto LMI = LogicalModules.begin(), LME = LogicalModules.end();
         LMI != LME; ++LMI) {
      if (LMI != LMH)
        if (auto Symbol = findSymbolInLogicalModule(LMI, Name, false))
          return Symbol;
    }

    return nullptr;
  }

  llvm::orc::JITSymbol findSymbol(const std::string &Name, bool ExportedSymbolsOnly) {
    for (auto LMI = LogicalModules.begin(), LME = LogicalModules.end();
         LMI != LME; ++LMI)
      if (auto Sym = findSymbolInLogicalModule(LMI, Name, ExportedSymbolsOnly))
        return Sym;
    return nullptr;
  }
  
  LogicalDylibResources& getDylibResources() { return DylibResources; }
  
protected:
  CompileLayer BaseLayer;
  LogicalModuleList LogicalModules;
  LogicalDylibResources DylibResources;
};

class MullJIT {
private:

  llvm::orc::SimpleCompiler compiler;
  LinkingLayer linkingLayer;
  CompileLayer compileLayer;

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

  typedef typename CompileLayer::ModuleSetHandleT BaseLayerModuleSetHandleT;

  // Provide type-erasure for the Modules and MemoryManagers.
  template <typename ResourceT>
  class ResourceOwner {
  public:
    ResourceOwner() = default;
    ResourceOwner(const ResourceOwner&) = delete;
    ResourceOwner& operator=(const ResourceOwner&) = delete;
    virtual ~ResourceOwner() { }
    virtual ResourceT& getResource() const = 0;
  };

  template <typename ResourceT, typename ResourcePtrT>
  class ResourceOwnerImpl : public ResourceOwner<ResourceT> {
  public:
    ResourceOwnerImpl(ResourcePtrT ResourcePtr)
    : ResourcePtr(std::move(ResourcePtr)) {}
    ResourceT& getResource() const override { return *ResourcePtr; }
  private:
    ResourcePtrT ResourcePtr;
  };

  template <typename ResourceT, typename ResourcePtrT>
  std::unique_ptr<ResourceOwner<ResourceT>>
  wrapOwnership(ResourcePtrT ResourcePtr) {
    typedef ResourceOwnerImpl<ResourceT, ResourcePtrT> RO;
    return llvm::make_unique<RO>(std::move(ResourcePtr));
  }

  struct LogicalModuleResources {
    llvm::Module *SourceModule;
    std::set<const llvm::Function *> StubsToClone;
    std::unique_ptr<llvm::orc::IndirectStubsManager> StubsMgr;

    LogicalModuleResources() = default;

    // Explicit move constructor to make MSVC happy.
    LogicalModuleResources(LogicalModuleResources &&Other)
    : SourceModule(std::move(Other.SourceModule)),
    StubsToClone(std::move(Other.StubsToClone)),
    StubsMgr(std::move(Other.StubsMgr)) {}

    // Explicit move assignment to make MSVC happy.
    LogicalModuleResources& operator=(LogicalModuleResources &&Other) {
      SourceModule = std::move(Other.SourceModule);
      StubsToClone = std::move(Other.StubsToClone);
      StubsMgr = std::move(Other.StubsMgr);
      return *this;
    }

    llvm::orc::JITSymbol findSymbol(llvm::StringRef Name, bool ExportedSymbolsOnly) {
      if (Name.endswith("$stub_ptr") && !ExportedSymbolsOnly) {
        assert(!ExportedSymbolsOnly && "Stubs are never exported");
        return StubsMgr->findPointer(Name.drop_back(9));
      }
      return StubsMgr->findStub(Name, ExportedSymbolsOnly);
    }

  };

  struct LogicalDylibResources {
    typedef std::function<llvm::RuntimeDyld::SymbolInfo(const std::string&)>
    SymbolResolverFtor;

    typedef std::function<typename CompileLayer::ModuleSetHandleT(
                                                                CompileLayer&,
                                                                std::unique_ptr<llvm::Module>,
                                                                std::unique_ptr<llvm::RuntimeDyld::SymbolResolver>)>
    ModuleAdderFtor;

    LogicalDylibResources() = default;

    // Explicit move constructor to make MSVC happy.
    LogicalDylibResources(LogicalDylibResources &&Other)
    : ExternalSymbolResolver(std::move(Other.ExternalSymbolResolver)),
    MemMgr(std::move(Other.MemMgr)),
    ModuleAdder(std::move(Other.ModuleAdder)) {}

    // Explicit move assignment operator to make MSVC happy.
    LogicalDylibResources& operator=(LogicalDylibResources &&Other) {
      ExternalSymbolResolver = std::move(Other.ExternalSymbolResolver);
      MemMgr = std::move(Other.MemMgr);
      ModuleAdder = std::move(Other.ModuleAdder);
      return *this;
    }

    std::unique_ptr<llvm::RuntimeDyld::SymbolResolver> ExternalSymbolResolver;
    std::unique_ptr<ResourceOwner<llvm::RuntimeDyld::MemoryManager>> MemMgr;
    ModuleAdderFtor ModuleAdder;
  };

  typedef MullLogicalDylib<LogicalModuleResources,
  LogicalDylibResources> CODLogicalDylib;

  typedef typename CODLogicalDylib::LogicalModuleHandle LogicalModuleHandle;
public:

  /// @brief Construct a compile-on-demand layer instance.
  MullJIT(llvm::TargetMachine &machine);

  void addModuleSet(std::vector<llvm::Module *> &modules,
                    std::unique_ptr<SectionMemoryManager> memoryManager,
                    std::unique_ptr<RuntimeDyld::SymbolResolver> resolver);

  /// @brief Search for the given named symbol.
  /// @param Name The name of the symbol to search for.
  /// @param ExportedSymbolsOnly If true, search only for exported symbols.
  /// @return A handle for the given named symbol, if it exists.
  llvm::orc::JITSymbol findSymbol(StringRef Name, bool ExportedSymbolsOnly);

#pragma mark - Call Tree Begin

  void prepareForExecution();
  std::unique_ptr<CallTree> createCallTree();
  void cleanupCallTree(std::unique_ptr<CallTree> root);

#pragma mark - Call Tree End

private:
  void addLogicalModule(llvm::Module *module);

  static std::string mangle(llvm::StringRef Name, const llvm::DataLayout &DL);

  llvm::orc::TargetAddress extractAndCompile(LogicalModuleHandle LMH,
                                             Function &F,
                                             uint64_t functionIndex);

  BaseLayerModuleSetHandleT emitFunction(LogicalModuleHandle LMH,
                                         Function &function,
                                         uint64_t functionIndex);

  void insertCallTreeCallbacks(Function &function, uint64_t index);

  std::unique_ptr<llvm::orc::JITCompileCallbackManager> callbackManager;
  std::function<std::unique_ptr<llvm::orc::IndirectStubsManager>()> stubsManagerBuilder;

  CODLogicalDylib logicalDylib;
  std::vector<CallTreeFunction> functions;
  DynamicCallTree dynamicCallTree;
};

}
