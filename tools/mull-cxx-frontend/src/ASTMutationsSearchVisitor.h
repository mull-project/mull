#pragma once

#include "ASTMutation.h"

#include <clang/AST/RecursiveASTVisitor.h>

class ASTMutationsSearchVisitor : public clang::RecursiveASTVisitor<ASTMutationsSearchVisitor> {
  std::vector<ASTMutation> astMutations;
  std::unordered_set<mull::MutatorKind> mutationsChecklist;

public:
  ASTMutationsSearchVisitor(std::unordered_set<mull::MutatorKind> mutationsChecklist)
      : astMutations(), mutationsChecklist(mutationsChecklist) {}

  std::vector<ASTMutation> getAstMutations();

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);

private:
  bool isValidMutation(mull::MutatorKind mutatorKind);
};
