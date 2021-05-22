#include "MullASTMutator.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>

void MullASTMutator::instrumentTranslationUnit() {
  _instrumentation.instrumentTranslationUnit();
}

void MullASTMutator::performUnaryOperatorOpcodeMutation(
    ASTMutationPoint &mutation, UnaryOperatorOpcodeMutation &unaryOperatorOpcodeMutator) {
  clang::UnaryOperator *oldUnaryOperator =
      clang::dyn_cast_or_null<clang::UnaryOperator>(mutation.mutableStmt);
  clang::UnaryOperator *newUnaryOperator =
      _factory.createUnaryOperator(unaryOperatorOpcodeMutator.replacementOpCode,
                                   oldUnaryOperator->getSubExpr(),
                                   oldUnaryOperator->getType(),
                                   oldUnaryOperator->getValueKind());

  _clangAstMutator.replaceExpression(
      oldUnaryOperator, newUnaryOperator, mutation.mutationIdentifier);
  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}

void MullASTMutator::performUnaryOperatorRemovalMutation(
    ASTMutationPoint &mutation, UnaryOperatorRemovalMutation &unaryNotToNoopMutator) {

  _clangAstMutator.replaceExpression(unaryNotToNoopMutator.unaryOperator,
                                     unaryNotToNoopMutator.unaryOperator->getSubExpr(),
                                     mutation.mutationIdentifier);

  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}

void MullASTMutator::performBinaryMutation(ASTMutationPoint &mutation,
                                           BinaryMutation &binaryMutator) {
  clang::BinaryOperator *oldBinaryOperator =
      clang::dyn_cast<clang::BinaryOperator>(mutation.mutableStmt);

  clang::BinaryOperator *newBinaryOperator;

  if (clang::CompoundAssignOperator *compoundAssignOperator =
          clang::dyn_cast_or_null<clang::CompoundAssignOperator>(oldBinaryOperator)) {
    newBinaryOperator =
        _factory.createCompoundAssignOperator(binaryMutator.replacementOpCode,
                                              oldBinaryOperator->getLHS(),
                                              oldBinaryOperator->getRHS(),
                                              oldBinaryOperator->getType(),
                                              oldBinaryOperator->getValueKind(),
                                              compoundAssignOperator->getComputationLHSType(),
                                              compoundAssignOperator->getComputationResultType());
  } else {
    newBinaryOperator = _factory.createBinaryOperator(binaryMutator.replacementOpCode,
                                                      oldBinaryOperator->getLHS(),
                                                      oldBinaryOperator->getRHS(),
                                                      oldBinaryOperator->getType(),
                                                      oldBinaryOperator->getValueKind());
  }

  _clangAstMutator.replaceExpression(
      oldBinaryOperator, newBinaryOperator, mutation.mutationIdentifier);
  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}

void MullASTMutator::performRemoveVoidMutation(ASTMutationPoint &mutation,
                                               RemoveVoidMutation &removeVoidMutator) {
  clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(mutation.mutableStmt);
  _clangAstMutator.replaceStatement(callExpr, nullptr, mutation.mutationIdentifier);

  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}

void MullASTMutator::performReplaceScalarMutation(
    ASTMutationPoint &mutation, ReplaceScalarCallMutation &replaceScalarCallMutator) {
  clang::CallExpr *callExpr = replaceScalarCallMutator.callExpr;

  clang::Expr *replacementLiteral;
  if (callExpr->getType() == _context.IntTy) {
    replacementLiteral = _factory.createIntegerLiteral(42);
  } else if (callExpr->getType() == _context.FloatTy) {
    replacementLiteral = _factory.createFloatLiteral(42.f);
  } else if (callExpr->getType() == _context.DoubleTy) {
    replacementLiteral = _factory.createFloatLiteral(42.0);
  } else {
    assert(0 && "Not implemented");
  }

  _clangAstMutator.replaceExpression(callExpr, replacementLiteral, mutation.mutationIdentifier);
  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}

void MullASTMutator::performReplaceNumericAssignmentMutation(
    ASTMutationPoint &mutation, ReplaceNumericAssignmentMutation &replaceNumericAssignmentMutator) {

  clang::Expr *oldAssignedExpr = replaceNumericAssignmentMutator.assignmentBinaryOperator->getRHS();

  clang::Expr *replacementLiteral;
  if (oldAssignedExpr->getType() == _context.IntTy) {
    replacementLiteral = _factory.createIntegerLiteral(42);
  } else if (oldAssignedExpr->getType() == _context.FloatTy) {
    replacementLiteral = _factory.createFloatLiteral(42.f);
  } else if (oldAssignedExpr->getType() == _context.DoubleTy) {
    replacementLiteral = _factory.createFloatLiteral(42.0);
  } else {
    assert(0 && "Not implemented");
  }

  _clangAstMutator.replaceExpression(
      replaceNumericAssignmentMutator.assignmentBinaryOperator->getRHS(),
      replacementLiteral,
      mutation.mutationIdentifier);
  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}

void MullASTMutator::performReplaceNumericInitAssignmentMutation(
    ASTMutationPoint &mutation,
    ReplaceNumericInitAssignmentMutation &replaceNumericInitAssignmentMutator) {
  assert(replaceNumericInitAssignmentMutator.varDecl->getKind() == clang::VarDecl::Kind::Var);

  clang::Expr *oldAssignedExpr = replaceNumericInitAssignmentMutator.varDecl->getInit();

  clang::Expr *replacementLiteral;
  if (oldAssignedExpr->getType() == _context.IntTy) {
    replacementLiteral = _factory.createIntegerLiteral(42);
  } else if (oldAssignedExpr->getType() == _context.FloatTy) {
    replacementLiteral = _factory.createFloatLiteral(42.f);
  } else if (oldAssignedExpr->getType() == _context.DoubleTy) {
    replacementLiteral = _factory.createFloatLiteral(42.0);
  } else {
    assert(0 && "Not implemented");
  }

  _clangAstMutator.replaceExpression(
      oldAssignedExpr, replacementLiteral, mutation.mutationIdentifier);
  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}
