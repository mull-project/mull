#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "SourceLocation.h"

namespace llvm {

class Function;
class Instruction;
class Value;
class Module;

} // namespace llvm

namespace irm {

class IRMutation;

}

namespace mull {

class Compiler;
class Mutator;
class Bitcode;
class Test;

/// \brief Container class that stores information needed to find
/// MutationPoints. We need the indexes of function, basic block and instruction
/// to find the mutation point in the clone of original module, when mutator is
/// to apply mutation in that clone.
class MutationPointAddress {
  int functionIndex;
  int basicBlockIndex;
  int instructionIndex;

  std::string identifier;

public:
  MutationPointAddress(int FnIndex, int BBIndex, int IIndex);

  int getFnIndex() const;
  int getBBIndex() const;
  int getIIndex() const;

  llvm::Instruction &findInstruction(llvm::Module *module) const;
  llvm::Instruction &findInstruction(llvm::Function *function) const;

  static MutationPointAddress addressFromInstruction(const llvm::Instruction *instruction);
};

class MutationPoint {
  Mutator *mutator;
  const MutationPointAddress address;
  Bitcode *bitcode;
  llvm::Function *originalFunction;
  llvm::Function *mutatedFunction;
  std::string diagnostics;
  std::string replacement;
  const SourceLocation sourceLocation;
  irm::IRMutation *irMutator;
  std::string userIdentifier;
  bool covered;

public:
  MutationPoint(Mutator *mutator, MutationPointAddress address, Bitcode *bitcode,
                llvm::Function *originalFunction, llvm::Function *mutatedFunction,
                std::string diagnostics, std::string replacement, SourceLocation sourceLocation,
                irm::IRMutation *irMutator);
  MutationPoint(Mutator *mutator, irm::IRMutation *irMutator, llvm::Instruction *instruction,
                std::string replacement, Bitcode *m, std::string diagnostics);

  ~MutationPoint() = default;

  void setCovered(bool isCovered);
  bool isCovered();

  Mutator *getMutator();
  Mutator *getMutator() const;

  const std::string &getUserIdentifier() const;

  const MutationPointAddress &getAddress() const;
  llvm::Value *getOriginalValue() const;
  Bitcode *getBitcode() const;

  llvm::Function *getOriginalFunction();
  void setMutatedFunction(llvm::Function *function);
  llvm::Function *getMutatedFunction() const;

  const SourceLocation &getSourceLocation() const;

  void applyMutation();

  std::string getMutatorIdentifier() const;

  const std::string &getDiagnostics();
  const std::string &getDiagnostics() const;

  const std::string &getReplacement();

  std::string getMutatedFunctionName();
  std::string getOriginalFunctionName() const;

  std::string dump() const;
  std::string dumpSourceCodeContext() const;
};

} // namespace mull
