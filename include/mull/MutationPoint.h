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

  std::string getIdentifier() const;

  llvm::Instruction &findInstruction(llvm::Module *module) const;
  llvm::Instruction &findInstruction(llvm::Function *function) const;

  const static MutationPointAddress
  addressFromInstruction(const llvm::Instruction *instruction);
};

class MutationPoint {
  Mutator *mutator;
  const MutationPointAddress address;
  Bitcode *bitcode;
  llvm::Function *originalFunction;
  llvm::Function *mutatedFunction;
  std::string uniqueIdentifier;
  std::string diagnostics;
  std::string replacement;
  const SourceLocation sourceLocation;
  std::vector<std::pair<Test *, int>> reachableTests;
  irm::IRMutation *irMutator;

public:
  MutationPoint(Mutator *mutator, irm::IRMutation *irMutator,
                llvm::Instruction *instruction, std::string replacement,
                Bitcode *m, std::string diagnostics);

  ~MutationPoint() = default;

  Mutator *getMutator();
  const MutationPointAddress &getAddress() const;
  llvm::Value *getOriginalValue() const;
  Bitcode *getBitcode() const;

  llvm::Function *getOriginalFunction();
  void setMutatedFunction(llvm::Function *function);

  const SourceLocation &getSourceLocation() const;
  const std::string getSourceFileName() const;

  void addReachableTest(Test *test, int distance);
  void applyMutation();

  const std::vector<std::pair<Test *, int>> &getReachableTests() const;

  std::string getUniqueIdentifier();
  std::string getUniqueIdentifier() const;

  const std::string &getDiagnostics();
  const std::string &getDiagnostics() const;

  const std::string &getReplacement();

  std::string getTrampolineName();
  std::string getMutatedFunctionName();
  std::string getOriginalFunctionName() const;
};

} // namespace mull
