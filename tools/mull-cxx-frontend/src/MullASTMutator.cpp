#include "MullASTMutator.h"

#include <clang/AST/Expr.h>

void MullASTMutator::instrumentTranslationUnit() {
  _instrumentation.instrumentTranslationUnit();
}

void MullASTMutator::performMutations(std::vector<ASTMutation> &astMutations) {
  for (ASTMutation &astMutation : astMutations) {
    if (astMutation.mutationType == mull::MutatorKind::CXX_AddToSub) {
      clang::BinaryOperator *oldBinaryOperator =
          clang::dyn_cast<clang::BinaryOperator>(astMutation.mutableStmt);
      clang::BinaryOperator *newBinaryOperator =
          _factory.createBinaryOperator(clang::BinaryOperator::Opcode::BO_Sub,
                                        oldBinaryOperator->getLHS(),
                                        oldBinaryOperator->getRHS(),
                                        oldBinaryOperator->getType(),
                                        oldBinaryOperator->getValueKind());
      _clangAstMutator.replaceExpression(
          oldBinaryOperator, newBinaryOperator, astMutation.mutationIdentifier);
    } else if (astMutation.mutationType == mull::MutatorKind::CXX_SubToAdd) {
      clang::BinaryOperator *oldBinaryOperator =
          clang::dyn_cast<clang::BinaryOperator>(astMutation.mutableStmt);
      clang::BinaryOperator *newBinaryOperator =
          _factory.createBinaryOperator(clang::BinaryOperator::Opcode::BO_Add,
                                        oldBinaryOperator->getLHS(),
                                        oldBinaryOperator->getRHS(),
                                        oldBinaryOperator->getType(),
                                        oldBinaryOperator->getValueKind());
      _clangAstMutator.replaceExpression(
          oldBinaryOperator, newBinaryOperator, astMutation.mutationIdentifier);
    } else if (astMutation.mutationType == mull::MutatorKind::CXX_Logical_OrToAnd) {
      clang::BinaryOperator *oldBinaryOperator =
          clang::dyn_cast<clang::BinaryOperator>(astMutation.mutableStmt);
      clang::BinaryOperator *newBinaryOperator =
          _factory.createBinaryOperator(clang::BinaryOperator::Opcode::BO_LAnd,
                                        oldBinaryOperator->getLHS(),
                                        oldBinaryOperator->getRHS(),
                                        oldBinaryOperator->getType(),
                                        oldBinaryOperator->getValueKind());
      _clangAstMutator.replaceExpression(
          oldBinaryOperator, newBinaryOperator, astMutation.mutationIdentifier);
    } else if (astMutation.mutationType == mull::MutatorKind::CXX_RemoveVoidCall) {
      clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(astMutation.mutableStmt);
      _clangAstMutator.replaceStatement(callExpr, nullptr, astMutation.mutationIdentifier);
    } else {
      /// continue;
      assert(0 && "Not implemented");
    }
    _instrumentation.addMutantStringDefinition(astMutation.mutationIdentifier,
                                               static_cast<int>(astMutation.mutationType),
                                               astMutation.line,
                                               astMutation.column);
  }
}
