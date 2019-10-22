//
// Created by Stanislav Pankevich on 2019-10-13.
//

#pragma once

#include "mull/AST/ASTMutations.h"
#include "mull/AST/ASTTypes.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/MutationFilters/FilePathFilter.h"

#include <clang/AST/RecursiveASTVisitor.h>

#include <iostream>

namespace mull {

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
  mull::ThreadSafeASTUnit &astUnit;
  mull::ASTMutations &mutations;
  clang::SourceManager &sourceManager;
  mull::FilePathFilter &filePathFilter;
  mull::TraverseMask traverseMask;
  bool shouldSkipCurrentFunction;
public:
  explicit ASTVisitor(mull::ThreadSafeASTUnit &astUnit,
                      mull::ASTMutations &mutations,
                      mull::FilePathFilter &filePathFilter,
                      mull::TraverseMask traverseMask);

  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitFunctionDecl(clang::FunctionDecl* Decl);
  bool VisitExpr(clang::Expr *expr);

  void traverse();
private:
  void saveMutationPoint(const clang::Stmt *stmt, clang::SourceLocation location);
};

} // namespace mull
