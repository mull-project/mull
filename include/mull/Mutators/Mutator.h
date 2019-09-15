#pragma once

#include <string>
#include <vector>

namespace llvm {
class Function;
class Value;
class Module;
class Instruction;
} // namespace llvm

namespace irm {

class IRMutation;

}

namespace mull {

class Bitcode;
class MutationPoint;
class MutationPointAddress;
struct SourceLocation;

enum class MutatorKind {
  MathAddMutator,
  MathDivMutator,
  MathSubMutator,
  MathMulMutator,
  NegateMutator,
  RemoveVoidFunctionMutator,
  ReplaceCallMutator,
  AndOrReplacementMutator,
  ScalarValueMutator,
  ReplaceAssignmentMutator,

  CXX_Relation_LessThanToLessOrEqual,
  CXX_Relation_LessOrEqualToLessThan,
  CXX_Relation_GreaterThanToGreaterOrEqual,
  CXX_Relation_GreaterOrEqualToGreaterThan,
};

class Mutator {
public:
  virtual std::string getUniqueIdentifier() = 0;
  virtual std::string getUniqueIdentifier() const = 0;
  virtual MutatorKind mutatorKind() = 0;

  virtual std::string getDescription() const = 0;

  virtual void applyMutation(llvm::Function *function,
                             const MutationPointAddress &address,
                             irm::IRMutation *lowLevelMutation) = 0;
  virtual std::vector<MutationPoint *>
  getMutations(Bitcode *bitcode, llvm::Function *function) = 0;

  virtual ~Mutator() = default;
};

} // namespace mull
