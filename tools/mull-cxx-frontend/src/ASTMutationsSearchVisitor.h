#pragma once

#include "ASTMutation.h"

#include <clang/AST/RecursiveASTVisitor.h>

class ASTMutationsSearchVisitor : public clang::RecursiveASTVisitor<ASTMutationsSearchVisitor> {
  clang::ASTContext &_context;
  clang::SourceManager &sourceManager;
  std::vector<std::unique_ptr<ASTMutation>> astMutations;
  std::unordered_set<mull::MutatorKind> mutationsChecklist;

public:
  ASTMutationsSearchVisitor(clang::ASTContext &context,
                            std::unordered_set<mull::MutatorKind> mutationsChecklist)
      : _context(context), sourceManager(context.getSourceManager()), astMutations(),
        mutationsChecklist(mutationsChecklist) {}

  std::vector<std::unique_ptr<ASTMutation>> &getAstMutations();

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitCallExpr(clang::CallExpr *callExpr);

private:
  bool isValidMutation(mull::MutatorKind mutatorKind);
  void recordMutationPoint(mull::MutatorKind mutatorKind, std::unique_ptr<Mutator> mutation,
                           clang::Stmt *stmt, clang::SourceLocation location);
};
