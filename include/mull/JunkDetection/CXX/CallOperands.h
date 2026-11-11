#pragma once

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>

#include <vector>

/// Header-only: shared with mull-rewriter, which does not link libmull
namespace mull {

inline void collectOperand(const clang::Expr *expression,
                           std::vector<const clang::Expr *> &operands) {
  const clang::Expr *inner = expression->IgnoreImplicit();
  // A braced initializer cannot be discarded as is, its elements can
  if (auto list = llvm::dyn_cast<clang::InitListExpr>(inner)) {
    for (const clang::Expr *init : list->inits()) {
      collectOperand(init, operands);
    }
    return;
  }
  if (auto list = llvm::dyn_cast<clang::CXXStdInitializerListExpr>(inner)) {
    collectOperand(list->getSubExpr(), operands);
    return;
  }
  if (auto construct = llvm::dyn_cast<clang::CXXConstructExpr>(inner)) {
    if (construct->isListInitialization() && !llvm::isa<clang::CXXTemporaryObjectExpr>(construct)) {
      for (const clang::Expr *argument : construct->arguments()) {
        collectOperand(argument, operands);
      }
      return;
    }
  }
  operands.push_back(expression);
}

/// The object and arguments of a call: the IR frontend replaces just the call, they are still
/// evaluated
inline std::vector<const clang::Expr *> callOperands(const clang::CallExpr *call) {
  std::vector<const clang::Expr *> operands;
  if (auto member = llvm::dyn_cast<clang::CXXMemberCallExpr>(call)) {
    const clang::Expr *object = member->getImplicitObjectArgument();
    if (object && !object->isImplicitCXXThis()) {
      collectOperand(object, operands);
    }
  } else if (!call->getDirectCallee()) {
    // E.g. a function pointer, a named function is not an operand
    collectOperand(call->getCallee(), operands);
  }
  for (const clang::Expr *argument : call->arguments()) {
    // Default arguments have no source at the call
    if (!llvm::isa<clang::CXXDefaultArgExpr>(argument)) {
      collectOperand(argument, operands);
    }
  }
  return operands;
}

} // namespace mull
