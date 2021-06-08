#pragma once

namespace mull {
namespace cxx {

class ASTMutationPoint;
class BinaryMutation;
class RemoveVoidMutation;
class ReplaceScalarCallMutation;
class UnaryOperatorOpcodeMutation;
class UnaryOperatorRemovalMutation;
class ReplaceNumericAssignmentMutation;
class ReplaceNumericInitAssignmentMutation;

class ASTMutator {
public:
  virtual ~ASTMutator() {}

  virtual void performBinaryMutation(ASTMutationPoint &mutation,
                                     BinaryMutation &binaryMutation) = 0;
  virtual void performRemoveVoidMutation(ASTMutationPoint &mutation,
                                         RemoveVoidMutation &removeVoidMutation) = 0;
  virtual void
  performReplaceScalarMutation(ASTMutationPoint &mutation,
                               ReplaceScalarCallMutation &replaceScalarCallMutation) = 0;
  virtual void
  performUnaryOperatorOpcodeMutation(ASTMutationPoint &mutation,
                                     UnaryOperatorOpcodeMutation &unaryOperatorOpcodeMutation) = 0;

  virtual void performUnaryOperatorRemovalMutation(
      ASTMutationPoint &mutation, UnaryOperatorRemovalMutation &unaryOperatorRemovalMutation) = 0;
  virtual void performReplaceNumericAssignmentMutation(
      ASTMutationPoint &mutation,
      ReplaceNumericAssignmentMutation &replaceNumericAssignmentMutation) = 0;

  virtual void performReplaceNumericInitAssignmentMutation(
      ASTMutationPoint &mutation,
      ReplaceNumericInitAssignmentMutation &replaceNumericInitAssignmentMutation) = 0;
};

} // namespace cxx
} // namespace mull
