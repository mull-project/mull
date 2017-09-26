#include "MullJIT.h"
#include "MutationPoint.h"
#include "MutationOperator.h"

namespace mull {

uint64_t *_callTreeMapping = nullptr;
std::stack<uint64_t> _callstack;

extern "C" void mull_enterFunction(uint64_t functionIndex) {
  assert(_callTreeMapping);
  DynamicCallTree::enterFunction(functionIndex, _callTreeMapping, _callstack);
}

extern "C" void mull_leaveFunction(uint64_t functionIndex) {
  assert(_callTreeMapping);
  DynamicCallTree::leaveFunction(functionIndex, _callTreeMapping, _callstack);
}

void mull_moveFunctionBody(Function &OrigF, ValueToValueMapTy &VMap,
                           ValueMaterializer *Materializer,
                           Function *NewF) {
  assert(!OrigF.isDeclaration() && "Nothing to move");
  if (!NewF)
    NewF = cast<Function>(VMap[&OrigF]);
  else
    assert(VMap[&OrigF] == NewF && "Incorrect function mapping in VMap.");
  assert(NewF && "Function mapping missing from VMap.");
  assert(NewF->getParent() != OrigF.getParent() &&
      "moveFunctionBody should only be used to move bodies between "
          "modules.");

  SmallVector<ReturnInst *, 8> Returns; // Ignore returns cloned.
  CloneFunctionInto(NewF, &OrigF, VMap, /*ModuleLevelChanges=*/true, Returns,
                    "", nullptr, nullptr, Materializer);
}

MullJIT::MullJIT(llvm::TargetMachine &machine)
    : compiler(machine),
      compileLayer(linkingLayer, compiler),
      callbackManager(llvm::orc::createLocalCompileCallbackManager(machine.getTargetTriple(), 0)),
      stubsManagerBuilder(llvm::orc::createLocalIndirectStubsManagerBuilder(machine.getTargetTriple())),
      logicalDylib(compileLayer),
      dynamicCallTree(functions) {
  CallTreeFunction phonyRoot(nullptr);
  functions.push_back(phonyRoot);
}

void MullJIT::addModuleSet(std::vector<llvm::Module *> &modules, std::unique_ptr<SectionMemoryManager> memoryManager,
                           std::unique_ptr<RuntimeDyld::SymbolResolver> resolver) {
  auto &LDResources = logicalDylib.getDylibResources();

  LDResources.ExternalSymbolResolver = std::move(resolver);

  auto &MemMgrRef = *memoryManager;
  LDResources.MemMgr =
      wrapOwnership<RuntimeDyld::MemoryManager>(std::move(memoryManager));

  LDResources.ModuleAdder =
      [&MemMgrRef](CompileLayer &B, std::unique_ptr<Module> M,
                   std::unique_ptr<RuntimeDyld::SymbolResolver> R) {
        std::vector<std::unique_ptr<Module>> Ms;
        Ms.push_back(std::move(M));
        return B.addModuleSet(std::move(Ms), &MemMgrRef, std::move(R));
      };

  // Process each of the modules in this module set.
  for (auto &M : modules) {
    addLogicalModule(std::move(M));
  }

  prepareForExecution();
}

llvm::orc::JITSymbol MullJIT::findSymbol(StringRef Name, bool ExportedSymbolsOnly) {
  if (auto Symbol = logicalDylib.findSymbol(Name, ExportedSymbolsOnly))
    return Symbol;
  return compileLayer.findSymbol(Name, ExportedSymbolsOnly);
}

void MullJIT::prepareForExecution() {
  assert(_callTreeMapping == nullptr && "Called twice?");

  /// Creating a memory to be shared between child and parent.
  _callTreeMapping = (uint64_t *) mmap(NULL,
                                       sizeof(uint64_t) * functions.size(),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_ANONYMOUS,
                                       -1,
                                       0);
  memset(_callTreeMapping, 0, functions.size());
  dynamicCallTree.prepare(_callTreeMapping);
}

std::unique_ptr<CallTree> MullJIT::createCallTree() {
  assert(_callTreeMapping);
  return dynamicCallTree.createCallTree();
}

void MullJIT::cleanupCallTree(std::unique_ptr<CallTree> root) {
  dynamicCallTree.cleanupCallTree(std::move(root));
}

void MullJIT::addLogicalModule(llvm::Module *module) {

  // Bump the linkage and rename any anonymous/privote members in SrcM to
  // ensure that everything will resolve properly after we partition SrcM.
  orc::makeAllSymbolsExternallyAccessible(*module);

  // Create a logical module handle for SrcM within the logical dylib.
  auto LMH = logicalDylib.createLogicalModule();
  auto &LMResources = logicalDylib.getLogicalModuleResources(LMH);

  LMResources.SourceModule = module;

  // Create stub functions.
  const DataLayout &DL = module->getDataLayout();
  {
    typename llvm::orc::IndirectStubsManager::StubInitsMap StubInits;
    for (auto &F : *module) {
      // Skip declarations.

      if (F.isDeclaration())
        continue;

      LMResources.StubsToClone.insert(&F);

      // Create a callback, associate it with the stub for the function,
      // and set the compile action to compile the partition containing the
      // function.
      auto CCInfo = callbackManager->getCompileCallback();
      StubInits[mangle(F.getName(), DL)] =
          std::make_pair(CCInfo.getAddress(),
                         JITSymbolBase::flagsFromGlobalValue(F));

      CallTreeFunction callTreeFunction(&F);
      uint64_t index = functions.size();
      functions.push_back(callTreeFunction);
      CCInfo.setCompileAction([this, LMH, &F, index]() {
        return this->extractAndCompile(LMH, F, index);
      });
    }

    LMResources.StubsMgr = stubsManagerBuilder();
//      LMResources.StubsMgr = make_unique<orc::LocalIndirectStubsManager<orc::OrcX86_64_SysV>>();
    auto EC = LMResources.StubsMgr->createStubs(StubInits);
    (void) EC;
    // FIXME: This should be propagated back to the user. Stub creation may
    //        fail for remote JITs.
    assert(!EC && "Error generating stubs");
  }

  // If this module doesn't contain any globals or aliases we can bail out
  // early and avoid the overhead of creating and managing an empty globals
  // module.
  if (module->global_empty() && module->alias_empty())
    return;

  // Create the GlobalValues module.
  auto GVsM = llvm::make_unique<Module>((module->getName() + ".globals").str(),
                                        module->getContext());
  GVsM->setDataLayout(DL);

  ValueToValueMapTy VMap;

  // Clone global variable decls.
  for (auto &GV : module->globals())
    if (!GV.isDeclaration() && !VMap.count(&GV))
      llvm::orc::cloneGlobalVariableDecl(*GVsM, GV, &VMap);

  // And the aliases.
  for (auto &A : module->aliases())
    if (!VMap.count(&A))
      llvm::orc::cloneGlobalAliasDecl(*GVsM, A, VMap);

  // Now we need to clone the GV and alias initializers.

  // Initializers may refer to functions declared (but not defined) in this
  // module. Build a materializer to clone decls on demand.
  auto Materializer = createLambdaMaterializer(
      [this, &GVsM, &LMResources](Value *V) -> Value * {
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
  for (auto &GV : module->globals())
    if (!GV.isDeclaration())
      orc::moveGlobalVariableInitializer(GV, VMap, &Materializer);

  // Clone the global alias initializers.
  for (auto &A : module->aliases()) {
    auto *NewA = cast<GlobalAlias>(VMap[&A]);
    assert(NewA && "Alias not cloned?");
    Value *Init = MapValue(A.getAliasee(), VMap, RF_None, nullptr,
                           &Materializer);
    NewA->setAliasee(cast<Constant>(Init));
  }

  // Build a resolver for the globals module and add it to the base layer.
  auto GVsResolver = orc::createLambdaResolver(
      [this, LMH](const std::string &Name) {
        auto &LMResources = logicalDylib.getLogicalModuleResources(LMH);
        if (auto Sym = LMResources.StubsMgr->findStub(Name, false))
          return Sym.toRuntimeDyldSymbol();
        auto &LDResolver = logicalDylib.getDylibResources().ExternalSymbolResolver;
        return LDResolver->findSymbolInLogicalDylib(Name);
      },
      [this](const std::string &Name) {
        auto &LDResolver = logicalDylib.getDylibResources().ExternalSymbolResolver;
        return LDResolver->findSymbol(Name);
      });

  auto GVsH = logicalDylib.getDylibResources().ModuleAdder(compileLayer, std::move(GVsM),
                                                           std::move(GVsResolver));
  logicalDylib.addToLogicalModule(LMH, GVsH);
}

std::string MullJIT::mangle(llvm::StringRef Name, const llvm::DataLayout &DL) {
  std::string MangledName;
  {
    raw_string_ostream MangledNameStream(MangledName);
    Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
  }
  return MangledName;
}

llvm::orc::TargetAddress MullJIT::extractAndCompile(MullJIT::LogicalModuleHandle LMH,
                                                    Function &F,
                                                    uint64_t functionIndex) {
  auto &LMResources = logicalDylib.getLogicalModuleResources(LMH);
  Module &module = *LMResources.SourceModule;

  // If F is a declaration we must already have compiled it.
  if (F.isDeclaration())
    return 0;

  // Grab the name of the function being called here.
  std::string CalledFnName = mangle(F.getName(), module.getDataLayout());

  auto PartH = emitFunction(LMH, F, functionIndex);

  std::string FnName = mangle(F.getName(), module.getDataLayout());
  auto FnBodySym = compileLayer.findSymbolIn(PartH, FnName, false);
  assert(FnBodySym && "Couldn't find function body.");
  llvm::orc::TargetAddress FnBodyAddr = FnBodySym.getAddress();

  // Update the function body pointer for the stub.
  if (auto EC = LMResources.StubsMgr->updatePointer(FnName, FnBodyAddr))
    return 0;

  return FnBodyAddr;
}

MullJIT::BaseLayerModuleSetHandleT MullJIT::emitFunction(MullJIT::LogicalModuleHandle LMH,
                                                         Function &function,
                                                         uint64_t functionIndex) {
  auto &LMResources = logicalDylib.getLogicalModuleResources(LMH);
  Module &module = *LMResources.SourceModule;

  // Create the module.
  std::string NewName = module.getName();
  NewName += ".";
  NewName += function.getName();

  auto M = llvm::make_unique<Module>(NewName, module.getContext());
  M->setDataLayout(module.getDataLayout());
  ValueToValueMapTy VMap;

  auto Materializer = createLambdaMaterializer([this, &LMResources, &M,
                                                   &VMap, functionIndex](Value *V) -> Value * {
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
  mull_moveFunctionBody(function, VMap, &Materializer);

  // Create memory manager and symbol resolver.
  auto Resolver = orc::createLambdaResolver(
      [this, LMH](const std::string &Name) {
        if (auto Sym = logicalDylib.findSymbolInternally(LMH, Name))
          return Sym.toRuntimeDyldSymbol();
        auto &LDResolver = logicalDylib.getDylibResources().ExternalSymbolResolver;
        return LDResolver->findSymbolInLogicalDylib(Name);
      },
      [this](const std::string &Name) {
        auto &LDResolver = logicalDylib.getDylibResources().ExternalSymbolResolver;
        return LDResolver->findSymbol(Name);
      });

  insertCallTreeCallbacks(&module, &function, *M->getFunction(function.getName()), functionIndex);

  return logicalDylib.getDylibResources().ModuleAdder(compileLayer, std::move(M),
                                                      std::move(Resolver));
}

void MullJIT::insertCallTreeCallbacks(Module *module, Function *originalFunction, Function &function, uint64_t index) {
  if (function.isDeclaration()) {
    return;
  }

  if (currentMutationPoint) {
    if (currentMutationPoint->getModule() == module) {
      tryToApplyMutation(module, originalFunction, &function);
    }
  }

  auto &context = function.getParent()->getContext();
  auto parameterType = Type::getInt64Ty(context);
  auto voidType = Type::getVoidTy(context);
  std::vector<Type *> parameterTypes(1, parameterType);

  FunctionType *callbackType = FunctionType::get(voidType, parameterTypes, false);

  Value *functionIndex = ConstantInt::get(parameterType, index);
  std::vector<Value *> parameters(1, functionIndex);

  Function *enterFunction = Function::Create(callbackType,
                                             Function::ExternalLinkage,
                                             "mull_enterFunction",
                                             function.getParent());

  auto &entryBlock = *function.getBasicBlockList().begin();
  CallInst *enterFunctionCall = CallInst::Create(enterFunction, parameters);
  enterFunctionCall->insertBefore(&*entryBlock.getInstList().begin());

  Function *leaveFunction = Function::Create(callbackType,
                                             Function::ExternalLinkage,
                                             "mull_leaveFunction",
                                             function.getParent());

  for (auto &block : function.getBasicBlockList()) {
    ReturnInst *returnStatement = nullptr;
    if (!(returnStatement = dyn_cast<ReturnInst>(block.getTerminator()))) {
      continue;
    }

    CallInst *leaveFunctionCall = CallInst::Create(leaveFunction, parameters);
    leaveFunctionCall->insertBefore(returnStatement);
  }
}

void MullJIT::tryToApplyMutation(Module *module, Function *originalFunction, Function *function) {
  assert(currentMutationPoint);
  MutationPointAddress address = currentMutationPoint->getAddress();
  llvm::Function &functionWithMutation = *(std::next(module->begin(), address.getFnIndex()));
  if (originalFunction == &functionWithMutation) {
    currentMutationPoint->getOperator()->applyMutation(function, address);
  }
}

}

