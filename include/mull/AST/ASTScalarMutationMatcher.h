#pragma once

#include <clang/AST/ASTContext.h>

#include <iostream>

namespace mull {

/// Some scalar AST nodes cannot be traced directly instructions in LLVM IR.
/// Instead, they appear as the operands of the instructions that correspond to
/// their parents.
///
/// Example:
/// return 123 appears as   ret i32 123, !dbg !9 in LLVM IR
/// we record the location of the parent ReturnStmt of the IntegerLiteral
/// (=123) on the AST level and then we find 'ret' instruction in LLVM IR
/// which has 123 as its operand.
///
/// This class is supposed to be a dedicated place for finding such parents.
class ASTScalarMutationMatcher {
  clang::ASTContext &context;

public:
  explicit ASTScalarMutationMatcher(clang::ASTContext &context);

  bool isMutableExpr(const clang::Expr &expr, const clang::Stmt **mutableParentStmt,
                     clang::SourceLocation *mutationLocation);
  bool isMutableVarDeclExpr(const clang::Expr &expr, clang::SourceLocation *mutationLocation);
  bool isMutableAssignmentExpr(const clang::Expr &expr, clang::SourceLocation *mutationLocation);
};

} // namespace mull
