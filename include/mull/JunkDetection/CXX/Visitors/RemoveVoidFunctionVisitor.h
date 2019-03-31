#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class RemoveVoidFunctionVisitor
    : public clang::RecursiveASTVisitor<RemoveVoidFunctionVisitor> {
public:
  RemoveVoidFunctionVisitor(const VisitorParameters &parameters);

  bool VisitCallExpr(clang::CallExpr *callExpression);
  bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr *callExpression);
  bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr *callExpression);
  void handleCallExpr(clang::CallExpr *callExpression);
  bool foundMutant();

private:
  InstructionRangeVisitor visitor;
  const clang::ASTContext &astContext;
};

} // namespace mull