#pragma once

#include "ASTMutator.h"

#include <clang/AST/Expr.h>

namespace mull {
namespace cxx {

class ASTMutation {
public:
  ASTMutation(std::string replacement) : replacement(replacement) {}
  virtual void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) = 0;
  virtual ~ASTMutation() = default;
  const std::string getReplacement() const {
    return replacement;
  }

private:
  std::string replacement;
};

class BinaryMutation : public ASTMutation {
public:
  clang::BinaryOperator::Opcode replacementOpCode;

  BinaryMutation(clang::BinaryOperator::Opcode replacementOpCode, std::string replacement)
      : ASTMutation(replacement), replacementOpCode(replacementOpCode) {}
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performBinaryMutation(mutation, *this);
  }
  ~BinaryMutation() {}
};

class RemoveVoidMutation : public ASTMutation {
public:
  RemoveVoidMutation() : ASTMutation("") {}
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performRemoveVoidMutation(mutation, *this);
  }
  ~RemoveVoidMutation() {}
};

class ReplaceScalarCallMutation : public ASTMutation {
public:
  clang::CallExpr *callExpr;

  ReplaceScalarCallMutation(clang::CallExpr *callExpr) : ASTMutation("42"), callExpr(callExpr) {}
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
                              clang::UnaryOperator::Opcode replacementOpCode,
                              std::string replacement)
      : ASTMutation(replacement), unaryOperator(unaryOperator),
        replacementOpCode(replacementOpCode) {}

  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performUnaryOperatorOpcodeMutation(mutation, *this);
  }

  ~UnaryOperatorOpcodeMutation() {}
};

class UnaryOperatorRemovalMutation : public ASTMutation {
public:
  clang::UnaryOperator *unaryOperator;

  UnaryOperatorRemovalMutation(clang::UnaryOperator *unaryOperator)
      : ASTMutation(""), unaryOperator(unaryOperator) {}
  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performUnaryOperatorRemovalMutation(mutation, *this);
  }
  ~UnaryOperatorRemovalMutation() {}
};

class ReplaceNumericAssignmentMutation : public ASTMutation {

public:
  clang::BinaryOperator *assignmentBinaryOperator;

  ReplaceNumericAssignmentMutation(clang::BinaryOperator *assignmentBinaryOperator)
      : ASTMutation("42"), assignmentBinaryOperator(assignmentBinaryOperator) {}

  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performReplaceNumericAssignmentMutation(mutation, *this);
  }
};

class ReplaceNumericInitAssignmentMutation : public ASTMutation {

public:
  clang::VarDecl *varDecl;

  ReplaceNumericInitAssignmentMutation(clang::VarDecl *varDecl)
      : ASTMutation("42"), varDecl(varDecl) {}

  void performMutation(ASTMutationPoint &mutation, ASTMutator &mutator) {
    mutator.performReplaceNumericInitAssignmentMutation(mutation, *this);
  }
};

} // namespace cxx
} // namespace mull
