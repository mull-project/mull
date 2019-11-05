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
  clang::Expr *foundMutant();

private:
  bool validCallExpr(clang::CallExpr *callExpression,
                     clang::SourceLocation location);

  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
};

} // namespace mull