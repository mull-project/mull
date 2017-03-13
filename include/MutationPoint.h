#pragma once

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

namespace llvm {

class LLVMContext;
class Module;
class Value;

}

namespace mull {

class Compiler;
class MutationOperator;
class MullModule;

/// \brief Container class that stores information needed to find MutationPoints.
/// We need the indexes of function, basic block and instruction to find the mutation point
/// in the clone of original module, when mutation operator is to apply mutation in that clone.
class MutationPointAddress {
  int FnIndex;
  int BBIndex;
  int IIndex;

  std::string identifier;
public:
  MutationPointAddress(int FnIndex, int BBIndex, int IIndex) :
  FnIndex(FnIndex), BBIndex(BBIndex), IIndex(IIndex) {
    identifier = std::to_string(FnIndex) + "_" +
      std::to_string(BBIndex) + "_" +
      std::to_string(IIndex);
  }

  int getFnIndex() { return FnIndex; }
  int getBBIndex() { return BBIndex; }
  int getIIndex() { return IIndex; }

  std::string getIdentifier() {
    return identifier;
  }

  std::string getIdentifier() const {
    return identifier;
  }
};

class MutationPoint {
  MutationOperator *mutationOperator;
  MutationPointAddress Address;
  llvm::Value *OriginalValue;
  MullModule *module;
  std::string uniqueIdentifier;

public:
  MutationPoint(MutationOperator *op,
                MutationPointAddress Address,
                llvm::Value *Val,
                MullModule *m);
  ~MutationPoint();

  MutationOperator *getOperator();
  MutationPointAddress getAddress();
  llvm::Value *getOriginalValue();

  MutationOperator *getOperator() const;
  MutationPointAddress getAddress() const;
  llvm::Value *getOriginalValue() const;

  std::unique_ptr<llvm::Module> cloneModuleAndApplyMutation(llvm::LLVMContext &context);

  std::string getUniqueIdentifier();
  std::string getUniqueIdentifier() const;
};

}
