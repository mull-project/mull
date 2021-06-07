#pragma once

#include "ASTMutationPoint.h"

#include <clang/AST/RecursiveASTVisitor.h>

class MutationMap;

class ASTMutationsSearchVisitor : public clang::RecursiveASTVisitor<ASTMutationsSearchVisitor> {
  clang::ASTContext &context;
  clang::SourceManager &sourceManager;
  std::vector<std::unique_ptr<ASTMutationPoint>> astMutations;
  MutationMap &mutationMap;

public:
  ASTMutationsSearchVisitor(clang::ASTContext &context, MutationMap &mutationMap)
      : context(context), sourceManager(context.getSourceManager()), astMutations(),
        mutationMap(mutationMap) {}

  std::vector<std::unique_ptr<ASTMutationPoint>> &getAstMutations();

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitCallExpr(clang::CallExpr *callExpr);
  bool VisitVarDecl(clang::VarDecl *D);

private:
  bool isValidMutation(mull::MutatorKind mutatorKind);
  void recordMutationPoint(mull::MutatorKind mutatorKind, std::unique_ptr<ASTMutation> mutation,
                           clang::Stmt *stmt, clang::SourceLocation mutationLocation,
                           bool locationIsExpression);
  std::pair<clang::SourceLocation, clang::SourceLocation>
  getBeginEndMutationLocation(clang::Stmt *stmt, clang::SourceLocation mutationLocation,
                              bool locationIsExpression);
};
