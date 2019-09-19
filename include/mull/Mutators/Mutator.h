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
  NegateMutator,
  RemoveVoidFunctionMutator,
  ReplaceCallMutator,
  AndOrReplacementMutator,
  ScalarValueMutator,
  ReplaceAssignmentMutator,

  CXX_Arithmetic_AddToSub,
  CXX_Arithmetic_AddAssignToSubAssign,
  CXX_Arithmetic_PreIncToPreDec,
  CXX_Arithmetic_PostIncToPostDec,

  CXX_Arithmetic_SubToAdd,
  CXX_Arithmetic_SubAssignToAddAssign,
  CXX_Arithmetic_PreDecToPreInc,
  CXX_Arithmetic_PostDecToPostInc,

  CXX_Arithmetic_MulToDiv,
  CXX_Arithmetic_MulAssignToDivAssign,

  CXX_Arithmetic_DivToMul,
  CXX_Arithmetic_DivAssignToMulAssign,

  CXX_Relation_LessThanToLessOrEqual,
  CXX_Relation_LessOrEqualToLessThan,
  CXX_Relation_GreaterThanToGreaterOrEqual,
  CXX_Relation_GreaterOrEqualToGreaterThan,

  CXX_Relation_GreaterThanToLessOrEqual,
  CXX_Relation_GreaterOrEqualToLessThan,
  CXX_Relation_LessThanToGreaterOrEqual,
  CXX_Relation_LessOrEqualToGreaterThan,

  CXX_Relation_EqualToNotEqual,
  CXX_Relation_NotEqualToEqual,
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
