#pragma once

#include "ASTMutator.h"

#include <clang/AST/Expr.h>

namespace mull {
namespace cxx {

class ASTMutation {
public:
  virtual void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) = 0;
  virtual ~ASTMutation() {}
};

class BinaryMutation : public ASTMutation {
public:
  clang::BinaryOperator::Opcode replacementOpCode;

  BinaryMutation(clang::BinaryOperator::Opcode replacementOpCode)
      : replacementOpCode(replacementOpCode) {}
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performBinaryMutation(mutation, *this);
  }
  ~BinaryMutation() {}
};

class RemoveVoidMutation : public ASTMutation {
public:
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performRemoveVoidMutation(mutation, *this);
  }
  ~RemoveVoidMutation() {}
};

class ReplaceScalarCallMutation : public ASTMutation {
public:
  clang::CallExpr *callExpr;

  ReplaceScalarCallMutation(clang::CallExpr *callExpr) : callExpr(callExpr) {}
  ~ReplaceScalarCallMutation() {}
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performReplaceScalarMutation(mutation, *this);
  }
};

class UnaryOperatorOpcodeMutation : public ASTMutation {
public:
  clang::UnaryOperator *unaryOperator;
  clang::UnaryOperator::Opcode replacementOpCode;

  UnaryOperatorOpcodeMutation(clang::UnaryOperator *unaryOperator,
                              clang::UnaryOperator::Opcode replacementOpCode)
      : unaryOperator(unaryOperator), replacementOpCode(replacementOpCode) {}

  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performUnaryOperatorOpcodeMutation(mutation, *this);
  }

  ~UnaryOperatorOpcodeMutation() {}
};

class UnaryOperatorRemovalMutation : public ASTMutation {
public:
  clang::UnaryOperator *unaryOperator;

  UnaryOperatorRemovalMutation(clang::UnaryOperator *unaryOperator)
      : unaryOperator(unaryOperator) {}
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performUnaryOperatorRemovalMutation(mutation, *this);
  }
  ~UnaryOperatorRemovalMutation() {}
};

class ReplaceNumericAssignmentMutation : public ASTMutation {

public:
  clang::BinaryOperator *assignmentBinaryOperator;

  ReplaceNumericAssignmentMutation(clang::BinaryOperator *assignmentBinaryOperator)
      : assignmentBinaryOperator(assignmentBinaryOperator) {}

  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performReplaceNumericAssignmentMutation(mutation, *this);
  }
};

class ReplaceNumericInitAssignmentMutation : public ASTMutation {

public:
  clang::VarDecl *varDecl;

  ReplaceNumericInitAssignmentMutation(clang::VarDecl *varDecl) : varDecl(varDecl) {}

  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performReplaceNumericInitAssignmentMutation(mutation, *this);
  }
};

} // namespace cxx
} // namespace mull
