#pragma once

#include <string>
#include <vector>

namespace llvm {
class Function;
class Value;
class Module;
class Instruction;
}

namespace mull {

class Context;
class MullModule;
class MutationPoint;
class MutationPointAddress;
struct SourceLocation;

enum class MutatorKind {
  Unknown,
  ConditionalsBoundaryMutator,
  MathAddMutator,
  NegateMutator,
  RemoveVoidFunctionMutator
};

class Mutator {
public:
  virtual MutationPoint *getMutationPoint(MullModule *module,
                                          llvm::Function *function,
                                          llvm::Instruction *instruction,
                                          SourceLocation &sourceLocation,
                                          MutationPointAddress &address) = 0;

  virtual std::string getUniqueIdentifier() = 0;
  virtual std::string getUniqueIdentifier() const = 0;
  virtual MutatorKind mutatorKind()  { return MutatorKind::Unknown; }

  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *
  applyMutation(llvm::Function *function, MutationPointAddress &address) = 0;
  virtual ~Mutator() = default;
};

}
