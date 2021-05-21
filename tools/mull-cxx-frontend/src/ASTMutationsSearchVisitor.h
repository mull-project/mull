#pragma once

#include "ASTMutation.h"

#include <clang/AST/RecursiveASTVisitor.h>

class MutationMap;

class ASTMutationsSearchVisitor : public clang::RecursiveASTVisitor<ASTMutationsSearchVisitor> {
  clang::ASTContext &_context;
  clang::SourceManager &sourceManager;
  std::vector<std::unique_ptr<ASTMutation>> astMutations;
  MutationMap &mutationMap;

public:
  ASTMutationsSearchVisitor(clang::ASTContext &context, MutationMap &mutationMap)
      : _context(context), sourceManager(context.getSourceManager()), astMutations(),
        mutationMap(mutationMap) {}

  std::vector<std::unique_ptr<ASTMutation>> &getAstMutations();

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitCallExpr(clang::CallExpr *callExpr);
  bool VisitVarDecl(clang::VarDecl *D);
private:
  bool isValidMutation(mull::MutatorKind mutatorKind);
  void recordMutationPoint(mull::MutatorKind mutatorKind, std::unique_ptr<Mutator> mutation,
                           clang::Stmt *stmt, clang::SourceLocation location);
};
