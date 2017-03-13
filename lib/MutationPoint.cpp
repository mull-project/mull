#include "MutationPoint.h"
#include "Toolchain/Compiler.h"
#include "ModuleLoader.h"

#include "MutationOperators/MutationOperator.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace mull;
using namespace std;

static std::unique_ptr<Module> MullCloneModule(const Module *M,
                ValueToValueMapTy &VMap,
                function_ref<bool(const GlobalValue *)> ShouldCloneDefinition,
                LLVMContext &context) {
  // First off, we need to create the new module.
  std::unique_ptr<Module> New =
  llvm::make_unique<Module>(M->getModuleIdentifier(), context);
  New->setDataLayout(M->getDataLayout());
  New->setTargetTriple(M->getTargetTriple());
  New->setModuleInlineAsm(M->getModuleInlineAsm());

  // Loop over all of the global variables, making corresponding globals in the
  // new module.  Here we add them to the VMap and to the new Module.  We
  // don't worry about attributes or initializers, they will come later.
  //
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
       I != E; ++I) {
    GlobalVariable *GV = new GlobalVariable(*New,
                                            I->getValueType(),
                                            I->isConstant(), I->getLinkage(),
                                            (Constant*) nullptr, I->getName(),
                                            (GlobalVariable*) nullptr,
                                            I->getThreadLocalMode(),
                                            I->getType()->getAddressSpace());
    GV->copyAttributesFrom(&*I);
    VMap[&*I] = GV;
  }

  // Loop over the functions in the module, making external functions as before
  for (const Function &I : *M) {
    Function *NF = Function::Create(cast<FunctionType>(I.getValueType()),
                                    I.getLinkage(), I.getName(), New.get());
    NF->copyAttributesFrom(&I);
    VMap[&I] = NF;
  }

  // Loop over the aliases in the module
  for (Module::const_alias_iterator I = M->alias_begin(), E = M->alias_end();
       I != E; ++I) {
    if (!ShouldCloneDefinition(&*I)) {
      // An alias cannot act as an external reference, so we need to create
      // either a function or a global variable depending on the value type.
      // FIXME: Once pointee types are gone we can probably pick one or the
      // other.
      GlobalValue *GV;
      if (I->getValueType()->isFunctionTy())
        GV = Function::Create(cast<FunctionType>(I->getValueType()),
                              GlobalValue::ExternalLinkage, I->getName(),
                              New.get());
      else
        GV = new GlobalVariable(
                                *New, I->getValueType(), false, GlobalValue::ExternalLinkage,
                                (Constant *)nullptr, I->getName(), (GlobalVariable *)nullptr,
                                I->getThreadLocalMode(), I->getType()->getAddressSpace());
      VMap[&*I] = GV;
      // We do not copy attributes (mainly because copying between different
      // kinds of globals is forbidden), but this is generally not required for
      // correctness.
      continue;
    }
    auto *GA = GlobalAlias::create(I->getValueType(),
                                   I->getType()->getPointerAddressSpace(),
                                   I->getLinkage(), I->getName(), New.get());
    GA->copyAttributesFrom(&*I);
    VMap[&*I] = GA;
  }

  // Now that all of the things that global variable initializer can refer to
  // have been created, loop through and copy the global variable referrers
  // over...  We also set the attributes on the global now.
  //
  for (Module::const_global_iterator I = M->global_begin(), E = M->global_end();
       I != E; ++I) {
    if (I->isDeclaration())
      continue;

    GlobalVariable *GV = cast<GlobalVariable>(VMap[&*I]);
    if (!ShouldCloneDefinition(&*I)) {
      // Skip after setting the correct linkage for an external reference.
      GV->setLinkage(GlobalValue::ExternalLinkage);
      continue;
    }
    if (I->hasInitializer())
      GV->setInitializer(MapValue(I->getInitializer(), VMap));
  }

  // Similarly, copy over function bodies now...
  //
  for (const Function &I : *M) {
    if (I.isDeclaration())
      continue;

    Function *F = cast<Function>(VMap[&I]);
    if (!ShouldCloneDefinition(&I)) {
      // Skip after setting the correct linkage for an external reference.
      F->setLinkage(GlobalValue::ExternalLinkage);
      // Personality function is not valid on a declaration.
      F->setPersonalityFn(nullptr);
      continue;
    }

    Function::arg_iterator DestI = F->arg_begin();
    for (Function::const_arg_iterator J = I.arg_begin(); J != I.arg_end();
         ++J) {
      DestI->setName(J->getName());
      VMap[&*J] = &*DestI++;
    }

    SmallVector<ReturnInst *, 8> Returns; // Ignore returns cloned.
    CloneFunctionInto(F, &I, VMap, /*ModuleLevelChanges=*/true, Returns);

    if (I.hasPersonalityFn())
      F->setPersonalityFn(MapValue(I.getPersonalityFn(), VMap));
  }

  // And aliases
  for (Module::const_alias_iterator I = M->alias_begin(), E = M->alias_end();
       I != E; ++I) {
    // We already dealt with undefined aliases above.
    if (!ShouldCloneDefinition(&*I))
      continue;
    GlobalAlias *GA = cast<GlobalAlias>(VMap[&*I]);
    if (const Constant *C = I->getAliasee())
      GA->setAliasee(MapValue(C, VMap));
  }

  // And named metadata....
  for (Module::const_named_metadata_iterator I = M->named_metadata_begin(),
       E = M->named_metadata_end(); I != E; ++I) {
    const NamedMDNode &NMD = *I;
    NamedMDNode *NewNMD = New->getOrInsertNamedMetadata(NMD.getName());
    for (unsigned i = 0, e = NMD.getNumOperands(); i != e; ++i)
      NewNMD->addOperand(MapMetadata(NMD.getOperand(i), VMap));
  }
  
  return New;
}

static std::unique_ptr<Module> MullCloneModule(const Module *module, LLVMContext &context) {
  ValueToValueMapTy VMap;
  return MullCloneModule(module, VMap, [](const GlobalValue *GV) { return true; }, context);
}

MutationPoint::MutationPoint(MutationOperator *op,
                             MutationPointAddress Address,
                             Value *Val,
                             MullModule *m) :
  mutationOperator(op), Address(Address), OriginalValue(Val), module(m)
{
  string moduleID = module->getUniqueIdentifier();
  string addressID = Address.getIdentifier();
  string operatorID = mutationOperator->uniqueID();

  uniqueIdentifier = moduleID + "_" + addressID + "_" + operatorID;
}

MutationPoint::~MutationPoint() {}

MutationOperator *MutationPoint::getOperator() {
  return mutationOperator;
}

MutationPointAddress MutationPoint::getAddress() {
  return Address;
}

Value *MutationPoint::getOriginalValue() {
  return OriginalValue;
}

MutationOperator *MutationPoint::getOperator() const {
  return mutationOperator;
}

MutationPointAddress MutationPoint::getAddress() const {
  return Address;
}

Value *MutationPoint::getOriginalValue() const {
  return OriginalValue;
}

std::unique_ptr<llvm::Module>
MutationPoint::cloneModuleAndApplyMutation(LLVMContext &context) {
  auto copyForMutation = MullCloneModule(module->getModule(), context);

  mutationOperator->applyMutation(copyForMutation.get(), Address, *OriginalValue);

  return copyForMutation;
}

std::string MutationPoint::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MutationPoint::getUniqueIdentifier() const {
  return uniqueIdentifier;
}
