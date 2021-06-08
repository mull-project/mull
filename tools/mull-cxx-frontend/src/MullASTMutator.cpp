#include "MullASTMutator.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>

namespace mull {
namespace cxx {

void MullASTMutator::instrumentTranslationUnit() {
  instrumentation.instrumentTranslationUnit();
}

void MullASTMutator::performUnaryOperatorOpcodeMutation(
    ASTMutationPoint &mutation, UnaryOperatorOpcodeMutation &unaryOperatorOpcodeMutator) {
  clang::UnaryOperator *oldUnaryOperator =
      clang::dyn_cast_or_null<clang::UnaryOperator>(mutation.mutableStmt);
  clang::UnaryOperator *newUnaryOperator =
      factory.createUnaryOperator(unaryOperatorOpcodeMutator.replacementOpCode,
                                  oldUnaryOperator->getSubExpr(),
                                  oldUnaryOperator->getType(),
                                  oldUnaryOperator->getValueKind());

  clangAstMutator.replaceExpression(
      oldUnaryOperator, newUnaryOperator, mutation.mutationIdentifier);
  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

void MullASTMutator::performUnaryOperatorRemovalMutation(
    ASTMutationPoint &mutation, UnaryOperatorRemovalMutation &unaryNotToNoopMutator) {

  clangAstMutator.replaceExpression(unaryNotToNoopMutator.unaryOperator,
                                    unaryNotToNoopMutator.unaryOperator->getSubExpr(),
                                    mutation.mutationIdentifier);

  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

void MullASTMutator::performBinaryMutation(ASTMutationPoint &mutation,
                                           BinaryMutation &binaryMutator) {
  clang::BinaryOperator *oldBinaryOperator =
      clang::dyn_cast<clang::BinaryOperator>(mutation.mutableStmt);

  clang::BinaryOperator *newBinaryOperator;

  if (clang::CompoundAssignOperator *compoundAssignOperator =
          clang::dyn_cast_or_null<clang::CompoundAssignOperator>(oldBinaryOperator)) {
    newBinaryOperator =
        factory.createCompoundAssignOperator(binaryMutator.replacementOpCode,
                                             oldBinaryOperator->getLHS(),
                                             oldBinaryOperator->getRHS(),
                                             oldBinaryOperator->getType(),
                                             oldBinaryOperator->getValueKind(),
                                             compoundAssignOperator->getComputationLHSType(),
                                             compoundAssignOperator->getComputationResultType());
  } else {
    newBinaryOperator = factory.createBinaryOperator(binaryMutator.replacementOpCode,
                                                     oldBinaryOperator->getLHS(),
                                                     oldBinaryOperator->getRHS(),
                                                     oldBinaryOperator->getType(),
                                                     oldBinaryOperator->getValueKind());
  }

  clangAstMutator.replaceExpression(
      oldBinaryOperator, newBinaryOperator, mutation.mutationIdentifier);
  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

void MullASTMutator::performRemoveVoidMutation(ASTMutationPoint &mutation,
                                               RemoveVoidMutation &removeVoidMutator) {
  clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(mutation.mutableStmt);
  clangAstMutator.replaceStatement(callExpr, nullptr, mutation.mutationIdentifier);

  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

void MullASTMutator::performReplaceScalarMutation(
    ASTMutationPoint &mutation, ReplaceScalarCallMutation &replaceScalarCallMutator) {
  clang::CallExpr *callExpr = replaceScalarCallMutator.callExpr;

  clang::Expr *replacementLiteral;
  if (callExpr->getType() == context.IntTy) {
    replacementLiteral = factory.createIntegerLiteral(42);
  } else if (callExpr->getType() == context.FloatTy) {
    replacementLiteral = factory.createFloatLiteral(42.f);
  } else if (callExpr->getType() == context.DoubleTy) {
    replacementLiteral = factory.createFloatLiteral(42.0);
  } else {
    assert(0 && "Not implemented");
  }

  clangAstMutator.replaceExpression(callExpr, replacementLiteral, mutation.mutationIdentifier);
  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

void MullASTMutator::performReplaceNumericAssignmentMutation(
    ASTMutationPoint &mutation, ReplaceNumericAssignmentMutation &replaceNumericAssignmentMutator) {

  clang::Expr *oldAssignedExpr = replaceNumericAssignmentMutator.assignmentBinaryOperator->getRHS();

  clang::Expr *replacementLiteral;
  if (oldAssignedExpr->getType() == context.IntTy) {
    replacementLiteral = factory.createIntegerLiteral(42);
  } else if (oldAssignedExpr->getType() == context.FloatTy) {
    replacementLiteral = factory.createFloatLiteral(42.f);
  } else if (oldAssignedExpr->getType() == context.DoubleTy) {
    replacementLiteral = factory.createFloatLiteral(42.0);
  } else {
    assert(0 && "Not implemented");
  }

  clangAstMutator.replaceExpression(
      replaceNumericAssignmentMutator.assignmentBinaryOperator->getRHS(),
      replacementLiteral,
      mutation.mutationIdentifier);
  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

void MullASTMutator::performReplaceNumericInitAssignmentMutation(
    ASTMutationPoint &mutation,
    ReplaceNumericInitAssignmentMutation &replaceNumericInitAssignmentMutator) {
  assert(replaceNumericInitAssignmentMutator.varDecl->getKind() == clang::VarDecl::Kind::Var);

  clang::Expr *oldAssignedExpr = replaceNumericInitAssignmentMutator.varDecl->getInit();

  clang::Expr *replacementLiteral;
  if (oldAssignedExpr->getType() == context.IntTy) {
    replacementLiteral = factory.createIntegerLiteral(42);
  } else if (oldAssignedExpr->getType() == context.FloatTy) {
    replacementLiteral = factory.createFloatLiteral(42.f);
  } else if (oldAssignedExpr->getType() == context.DoubleTy) {
    replacementLiteral = factory.createFloatLiteral(42.0);
  } else {
    assert(0 && "Not implemented");
  }

  clangAstMutator.replaceExpression(
      oldAssignedExpr, replacementLiteral, mutation.mutationIdentifier);
  instrumentation.addMutantStringDefinition(mutation.mutationBinaryRecord,
                                            static_cast<int>(mutation.mutationType),
                                            mutation.beginLine,
                                            mutation.beginColumn);
}

} // namespace cxx
} // namespace mull
