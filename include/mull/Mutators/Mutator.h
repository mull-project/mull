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
class FunctionUnderTest;
struct SourceLocation;

enum class MutatorKind {
  NegateMutator,
  RemoveVoidFunctionMutator,
  ReplaceCallMutator,
  ScalarValueMutator,

  CXX_AddToSub,
  CXX_AddAssignToSubAssign,
  CXX_PreIncToPreDec,
  CXX_PostIncToPostDec,

  CXX_SubToAdd,
  CXX_SubAssignToAddAssign,
  CXX_PreDecToPreInc,
  CXX_PostDecToPostInc,

  CXX_MulToDiv,
  CXX_MulAssignToDivAssign,

  CXX_DivToMul,
  CXX_DivAssignToMulAssign,

  CXX_RemToDiv,
  CXX_RemAssignToDivAssign,

  CXX_BitwiseNotToNoop,
  CXX_UnaryMinusToNoop,

  CXX_LShiftToRShift,
  CXX_LShiftAssignToRShiftAssign,

  CXX_RShiftToLShift,
  CXX_RShiftAssignToLShiftAssign,

  CXX_Logical_AndToOr,
  CXX_Logical_OrToAnd,

  CXX_Bitwise_OrToAnd,
  CXX_Bitwise_OrAssignToAndAssign,
  CXX_Bitwise_AndToOr,
  CXX_Bitwise_AndAssignToOrAssign,
  CXX_Bitwise_XorToOr,
  CXX_Bitwise_XorAssignToOrAssign,

  CXX_LessThanToLessOrEqual,
  CXX_LessOrEqualToLessThan,
  CXX_GreaterThanToGreaterOrEqual,
  CXX_GreaterOrEqualToGreaterThan,

  CXX_GreaterThanToLessOrEqual,
  CXX_GreaterOrEqualToLessThan,
  CXX_LessThanToGreaterOrEqual,
  CXX_LessOrEqualToGreaterThan,

  CXX_EqualToNotEqual,
  CXX_NotEqualToEqual,

  CXX_AssignConst,
  CXX_InitConst,
};

class Mutator {
public:
  virtual std::string getUniqueIdentifier() = 0;
  virtual std::string getUniqueIdentifier() const = 0;
  virtual MutatorKind mutatorKind() = 0;

  virtual std::string getDescription() const = 0;

  virtual void applyMutation(llvm::Function *function, const MutationPointAddress &address,
                             irm::IRMutation *lowLevelMutation) = 0;
  virtual std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                                    const FunctionUnderTest &function) = 0;

  virtual ~Mutator() = default;
};

} // namespace mull

namespace std {

template <> struct std::hash<mull::MutatorKind> {
  std::size_t operator()(const mull::MutatorKind &k) const {
    return static_cast<std::size_t>(k);
  }
};

} // namespace std
