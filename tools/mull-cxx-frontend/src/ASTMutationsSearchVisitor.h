#pragma once

#include "ASTMutation.h"

#include <clang/AST/RecursiveASTVisitor.h>

class ASTMutationsSearchVisitor : public clang::RecursiveASTVisitor<ASTMutationsSearchVisitor> {
  std::vector<ASTMutation> astMutations;

public:
  ASTMutationsSearchVisitor() : astMutations() {}

  std::vector<ASTMutation> getAstMutations();

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
};
