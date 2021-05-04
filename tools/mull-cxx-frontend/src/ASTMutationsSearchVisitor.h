#pragma once

#include "ASTMutation.h"

#include <clang/AST/RecursiveASTVisitor.h>

class ASTMutationsSearchVisitor : public clang::RecursiveASTVisitor<ASTMutationsSearchVisitor> {
  clang::ASTContext &_context;
  clang::SourceManager &sourceManager;
  std::vector<ASTMutation> astMutations;
  std::unordered_set<mull::MutatorKind> mutationsChecklist;

public:
  ASTMutationsSearchVisitor(clang::ASTContext &context,
                            std::unordered_set<mull::MutatorKind> mutationsChecklist)
      : _context(context), sourceManager(context.getSourceManager()), astMutations(),
        mutationsChecklist(mutationsChecklist) {}

  std::vector<ASTMutation> getAstMutations();

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitCallExpr(clang::CallExpr *callExpr);

private:
  bool isValidMutation(mull::MutatorKind mutatorKind);
  void recordMutationPoint(mull::MutatorKind mutatorKind, clang::Stmt *stmt,
                           clang::SourceLocation location);
};
