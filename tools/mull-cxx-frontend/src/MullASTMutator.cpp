#include "MullASTMutator.h"

#include <clang/AST/Expr.h>

void MullASTMutator::instrumentTranslationUnit() {
  _instrumentation.instrumentTranslationUnit();
}

void MullASTMutator::performBinaryMutation(ASTMutation &mutation, BinaryMutator &binaryMutator) {
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

void MullASTMutator::performRemoveVoidMutation(ASTMutation &mutation,
                                               RemoveVoidMutator &removeVoidMutator) {
  clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(mutation.mutableStmt);
  _clangAstMutator.replaceStatement(callExpr, nullptr, mutation.mutationIdentifier);

  _instrumentation.addMutantStringDefinition(mutation.mutationIdentifier,
                                             static_cast<int>(mutation.mutationType),
                                             mutation.line,
                                             mutation.column);
}
