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
#include <list>
#include <memory>
#include <set>
#include <utility>

using namespace llvm;

/// @brief Custom Compile On Demand layer
template <typename BaseLayerT,
typename CompileCallbackMgrT = llvm::orc::JITCompileCallbackManager,
typename IndirectStubsMgrT = llvm::orc::IndirectStubsManager>
class MullLayer {
private:

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

  typedef typename BaseLayerT::ModuleSetHandleT BaseLayerModuleSetHandleT;

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
    std::unique_ptr<ResourceOwner<llvm::Module>> SourceModule;
    std::set<const llvm::Function *> StubsToClone;
    std::unique_ptr<IndirectStubsMgrT> StubsMgr;

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

    typedef std::function<typename BaseLayerT::ModuleSetHandleT(
                                                                BaseLayerT&,
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

  typedef llvm::orc::LogicalDylib<BaseLayerT, LogicalModuleResources,
  LogicalDylibResources> CODLogicalDylib;

  typedef typename CODLogicalDylib::LogicalModuleHandle LogicalModuleHandle;
  typedef std::list<CODLogicalDylib> LogicalDylibList;

public:

  /// @brief Handle to a set of loaded modules.
  typedef typename LogicalDylibList::iterator ModuleSetHandleT;

  /// @brief Module partitioning functor.
  typedef std::function<std::set<Function*>(Function&)> PartitioningFtor;

  /// @brief Builder for IndirectStubsManagers.
  typedef std::function<std::unique_ptr<IndirectStubsMgrT>()>
  IndirectStubsManagerBuilderT;

  /// @brief Construct a compile-on-demand layer instance.
  MullLayer(BaseLayerT &BaseLayer,
            CompileCallbackMgrT &CallbackMgr,
            IndirectStubsManagerBuilderT CreateIndirectStubsManager,
            bool CloneStubsIntoPartitions = true)
  : BaseLayer(BaseLayer),
  CompileCallbackMgr(CallbackMgr),
  CreateIndirectStubsManager(std::move(CreateIndirectStubsManager)),
  CloneStubsIntoPartitions(CloneStubsIntoPartitions) {}

  /// @brief Add a module to the compile-on-demand layer.
  template <typename ModuleSetT, typename MemoryManagerPtrT,
  typename SymbolResolverPtrT>
  ModuleSetHandleT addModuleSet(ModuleSetT Ms,
                                MemoryManagerPtrT MemMgr,
                                SymbolResolverPtrT Resolver) {

    LogicalDylibs.push_back(CODLogicalDylib(BaseLayer));
    auto &LDResources = LogicalDylibs.back().getDylibResources();

    LDResources.ExternalSymbolResolver = std::move(Resolver);

    auto &MemMgrRef = *MemMgr;
    LDResources.MemMgr =
      wrapOwnership<RuntimeDyld::MemoryManager>(std::move(MemMgr));

    LDResources.ModuleAdder =
      [&MemMgrRef](BaseLayerT &B, std::unique_ptr<Module> M,
                   std::unique_ptr<RuntimeDyld::SymbolResolver> R)
    {
      std::vector<std::unique_ptr<Module>> Ms;
      Ms.push_back(std::move(M));
      return B.addModuleSet(std::move(Ms), &MemMgrRef, std::move(R));
    };

    // Process each of the modules in this module set.
    for (auto &M : Ms)
      addLogicalModule(LogicalDylibs.back(), std::move(M));

    return std::prev(LogicalDylibs.end());
  }

  /// @brief Remove the module represented by the given handle.
  ///
  ///   This will remove all modules in the layers below that were derived from
  /// the module represented by H.
  void removeModuleSet(ModuleSetHandleT H) {
    LogicalDylibs.erase(H);
  }

  /// @brief Search for the given named symbol.
  /// @param Name The name of the symbol to search for.
  /// @param ExportedSymbolsOnly If true, search only for exported symbols.
  /// @return A handle for the given named symbol, if it exists.
  llvm::orc::JITSymbol findSymbol(StringRef Name, bool ExportedSymbolsOnly) {
    for (auto LDI = LogicalDylibs.begin(), LDE = LogicalDylibs.end();
         LDI != LDE; ++LDI)
      if (auto Symbol = findSymbolIn(LDI, Name, ExportedSymbolsOnly))
        return Symbol;
    return BaseLayer.findSymbol(Name, ExportedSymbolsOnly);
  }

  /// @brief Get the address of a symbol provided by this layer, or some layer
  ///        below this one.
  llvm::orc::JITSymbol findSymbolIn(ModuleSetHandleT H, const std::string &Name,
                                    bool ExportedSymbolsOnly) {
    return H->findSymbol(Name, ExportedSymbolsOnly);
  }

private:

  template <typename ModulePtrT>
  void addLogicalModule(CODLogicalDylib &LD, ModulePtrT SrcMPtr) {

    // Bump the linkage and rename any anonymous/privote members in SrcM to
    // ensure that everything will resolve properly after we partition SrcM.
    orc::makeAllSymbolsExternallyAccessible(*SrcMPtr);

    // Create a logical module handle for SrcM within the logical dylib.
    auto LMH = LD.createLogicalModule();
    auto &LMResources =  LD.getLogicalModuleResources(LMH);

    LMResources.SourceModule = wrapOwnership<Module>(std::move(SrcMPtr));

    Module &SrcM = LMResources.SourceModule->getResource();

    // Create stub functions.
    const DataLayout &DL = SrcM.getDataLayout();
    {
      typename IndirectStubsMgrT::StubInitsMap StubInits;
      for (auto &F : SrcM) {
        // Skip declarations.
        if (F.isDeclaration())
          continue;

        // Record all functions defined by this module.
        if (CloneStubsIntoPartitions)
          LMResources.StubsToClone.insert(&F);

        // Create a callback, associate it with the stub for the function,
        // and set the compile action to compile the partition containing the
        // function.
        auto CCInfo = CompileCallbackMgr.getCompileCallback();
        StubInits[mangle(F.getName(), DL)] =
        std::make_pair(CCInfo.getAddress(),
                       JITSymbolBase::flagsFromGlobalValue(F));
        CCInfo.setCompileAction([this, &LD, LMH, &F]() {
          return this->extractAndCompile(LD, LMH, F);
        });
      }

      LMResources.StubsMgr = CreateIndirectStubsManager();
      auto EC = LMResources.StubsMgr->createStubs(StubInits);
      (void)EC;
      // FIXME: This should be propagated back to the user. Stub creation may
      //        fail for remote JITs.
      assert(!EC && "Error generating stubs");
    }

    // If this module doesn't contain any globals or aliases we can bail out
    // early and avoid the overhead of creating and managing an empty globals
    // module.
    if (SrcM.global_empty() && SrcM.alias_empty())
      return;

    // Create the GlobalValues module.
    auto GVsM = llvm::make_unique<Module>((SrcM.getName() + ".globals").str(),
                                          SrcM.getContext());
    GVsM->setDataLayout(DL);

    ValueToValueMapTy VMap;

    // Clone global variable decls.
    for (auto &GV : SrcM.globals())
      if (!GV.isDeclaration() && !VMap.count(&GV))
        llvm::orc::cloneGlobalVariableDecl(*GVsM, GV, &VMap);

    // And the aliases.
    for (auto &A : SrcM.aliases())
      if (!VMap.count(&A))
        llvm::orc::cloneGlobalAliasDecl(*GVsM, A, VMap);

    // Now we need to clone the GV and alias initializers.

    // Initializers may refer to functions declared (but not defined) in this
    // module. Build a materializer to clone decls on demand.
    auto Materializer = createLambdaMaterializer(
                                                 [this, &GVsM, &LMResources](Value *V) -> Value* {
                                                   if (auto *F = dyn_cast<Function>(V)) {
                                                     // Decls in the original module just get cloned.
                                                     if (F->isDeclaration())
                                                       return llvm::orc::cloneFunctionDecl(*GVsM, *F);

                                                     // Definitions in the original module (which we have emitted stubs
                                                     // for at this point) get turned into a constant alias to the stub
                                                     // instead.
                                                     const DataLayout &DL = GVsM->getDataLayout();
                                                     std::string FName = mangle(F->getName(), DL);
                                                     auto StubSym = LMResources.StubsMgr->findStub(FName, false);
                                                     unsigned PtrBitWidth = DL.getPointerTypeSizeInBits(F->getType());
                                                     ConstantInt *StubAddr =
                                                     ConstantInt::get(GVsM->getContext(),
                                                                      APInt(PtrBitWidth, StubSym.getAddress()));
                                                     Constant *Init = ConstantExpr::getCast(Instruction::IntToPtr,
                                                                                            StubAddr, F->getType());
                                                     return GlobalAlias::create(F->getFunctionType(),
                                                                                F->getType()->getAddressSpace(),
                                                                                F->getLinkage(), F->getName(),
                                                                                Init, GVsM.get());
                                                   }
                                                   // else....
                                                   return nullptr;
                                                 });

    // Clone the global variable initializers.
    for (auto &GV : SrcM.globals())
      if (!GV.isDeclaration())
        orc::moveGlobalVariableInitializer(GV, VMap, &Materializer);

    // Clone the global alias initializers.
    for (auto &A : SrcM.aliases()) {
      auto *NewA = cast<GlobalAlias>(VMap[&A]);
      assert(NewA && "Alias not cloned?");
      Value *Init = MapValue(A.getAliasee(), VMap, RF_None, nullptr,
                             &Materializer);
      NewA->setAliasee(cast<Constant>(Init));
    }

    // Build a resolver for the globals module and add it to the base layer.
    auto GVsResolver = orc::createLambdaResolver(
                                                 [&LD, LMH](const std::string &Name) {
                                                   auto &LMResources = LD.getLogicalModuleResources(LMH);
                                                   if (auto Sym = LMResources.StubsMgr->findStub(Name, false))
                                                     return Sym.toRuntimeDyldSymbol();
                                                   auto &LDResolver = LD.getDylibResources().ExternalSymbolResolver;
                                                   return LDResolver->findSymbolInLogicalDylib(Name);
                                                 },
                                                 [&LD](const std::string &Name) {
                                                   auto &LDResolver = LD.getDylibResources().ExternalSymbolResolver;
                                                   return LDResolver->findSymbol(Name);
                                                 });

    auto GVsH = LD.getDylibResources().ModuleAdder(BaseLayer, std::move(GVsM),
                                                   std::move(GVsResolver));
    LD.addToLogicalModule(LMH, GVsH);
  }

  static std::string mangle(llvm::StringRef Name, const llvm::DataLayout &DL) {
    std::string MangledName;
    {
      raw_string_ostream MangledNameStream(MangledName);
      Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
    }
    return MangledName;
  }

  llvm::orc::TargetAddress extractAndCompile(CODLogicalDylib &LD,
                                             LogicalModuleHandle LMH,
                                             Function &F) {
    auto &LMResources = LD.getLogicalModuleResources(LMH);
    Module &SrcM = LMResources.SourceModule->getResource();

    // If F is a declaration we must already have compiled it.
    if (F.isDeclaration())
      return 0;

    // Grab the name of the function being called here.
    std::string CalledFnName = mangle(F.getName(), SrcM.getDataLayout());

    auto PartH = emitFunction(LD, LMH, F);

    std::string FnName = mangle(F.getName(), SrcM.getDataLayout());
    auto FnBodySym = BaseLayer.findSymbolIn(PartH, FnName, false);
    assert(FnBodySym && "Couldn't find function body.");
    llvm::orc::TargetAddress FnBodyAddr = FnBodySym.getAddress();

    // Update the function body pointer for the stub.
    if (auto EC = LMResources.StubsMgr->updatePointer(FnName, FnBodyAddr))
      return 0;

    return FnBodyAddr;
  }

  BaseLayerModuleSetHandleT emitFunction(CODLogicalDylib &LD,
                                         LogicalModuleHandle LMH,
                                         Function &function) {
    auto &LMResources = LD.getLogicalModuleResources(LMH);
    Module &SrcM = LMResources.SourceModule->getResource();

    SrcM.dump();

    // Create the module.
    std::string NewName = SrcM.getName();
    NewName += ".";
    NewName += function.getName();

    auto M = llvm::make_unique<Module>(NewName, SrcM.getContext());
    M->setDataLayout(SrcM.getDataLayout());
    ValueToValueMapTy VMap;

    auto Materializer = createLambdaMaterializer([this, &LMResources, &M,
                                                  &VMap](Value *V) -> Value * {
      if (auto *GV = dyn_cast<GlobalVariable>(V))
        return orc::cloneGlobalVariableDecl(*M, *GV);

      if (auto *F = dyn_cast<Function>(V)) {
        // Check whether we want to clone an available_externally definition.
        if (!LMResources.StubsToClone.count(F))
          return orc::cloneFunctionDecl(*M, *F);

        // Ok - we want an inlinable stub. For that to work we need a decl
        // for the stub pointer.
        auto *StubPtr = llvm::orc::createImplPointer(*F->getType(), *M,
                                                     F->getName() + "$stub_ptr", nullptr);
        auto *ClonedF = llvm::orc::cloneFunctionDecl(*M, *F);
        llvm::orc::makeStub(*ClonedF, *StubPtr);
        ClonedF->setLinkage(GlobalValue::AvailableExternallyLinkage);
        ClonedF->addFnAttr(Attribute::AlwaysInline);
        return ClonedF;
      }

      if (auto *A = dyn_cast<GlobalAlias>(V)) {
        auto *Ty = A->getValueType();
        if (Ty->isFunctionTy())
          return Function::Create(cast<FunctionType>(Ty),
                                  GlobalValue::ExternalLinkage, A->getName(),
                                  M.get());

        return new GlobalVariable(*M, Ty, false, GlobalValue::ExternalLinkage,
                                  nullptr, A->getName(), nullptr,
                                  GlobalValue::NotThreadLocal,
                                  A->getType()->getAddressSpace());
      }

      return nullptr;
    });

    // Create decls in the new module.
    orc::cloneFunctionDecl(*M, function, &VMap);

    // Move the function bodies.
    orc::moveFunctionBody(function, VMap, &Materializer);

    // Create memory manager and symbol resolver.
    auto Resolver = orc::createLambdaResolver(
                                              [this, &LD, LMH](const std::string &Name) {
                                                if (auto Sym = LD.findSymbolInternally(LMH, Name))
                                                  return Sym.toRuntimeDyldSymbol();
                                                auto &LDResolver = LD.getDylibResources().ExternalSymbolResolver;
                                                return LDResolver->findSymbolInLogicalDylib(Name);
                                              },
                                              [this, &LD](const std::string &Name) {
                                                auto &LDResolver = LD.getDylibResources().ExternalSymbolResolver;
                                                return LDResolver->findSymbol(Name);
                                              });

    emitCallTreeCallbacks(*M.get());

    return LD.getDylibResources().ModuleAdder(BaseLayer, std::move(M),
                                              std::move(Resolver));
  }

  void emitCallTreeCallbacks(Module &module) {
    module.dump();
  }

  BaseLayerT &BaseLayer;
  CompileCallbackMgrT &CompileCallbackMgr;
  IndirectStubsManagerBuilderT CreateIndirectStubsManager;

  LogicalDylibList LogicalDylibs;
  bool CloneStubsIntoPartitions;
};

class MullJIT {
  typedef llvm::orc::ObjectLinkingLayer<> LinkingLayer;
  typedef llvm::orc::IRCompileLayer<LinkingLayer> CompileLayer;
  typedef MullLayer<CompileLayer> CODLayer;

  std::unique_ptr<llvm::orc::JITCompileCallbackManager> callbackManager;
  std::function<std::unique_ptr<llvm::orc::IndirectStubsManager>()> stubsManager;

  llvm::orc::SimpleCompiler compiler;
  LinkingLayer linkingLayer;
  CompileLayer compileLayer;
  CODLayer compileOnDemandLayer;

public:
  MullJIT(llvm::TargetMachine &machine) :
    callbackManager(llvm::orc::createLocalCompileCallbackManager(machine.getTargetTriple(), 0)),
    stubsManager(llvm::orc::createLocalIndirectStubsManagerBuilder(machine.getTargetTriple())),
    compiler(machine),
    compileLayer(linkingLayer, compiler),
    compileOnDemandLayer(compileLayer, *(callbackManager.get()), stubsManager) {
  }

  CODLayer &jit() { return compileOnDemandLayer; }
};
