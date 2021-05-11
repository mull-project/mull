#pragma once

#include <clang/AST/Expr.h>
#include <clang/AST/OperationKinds.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Specifiers.h>

extern const clang::SourceLocation NULL_LOCATION;

class ASTNodeFactory {
  clang::ASTContext &_context;

public:
  ASTNodeFactory(clang::ASTContext &context) : _context(context) {}

  clang::IntegerLiteral *createIntegerLiteral(int value);
  clang::StringLiteral *createStringLiteral(std::string value);
  clang::IfStmt *createIfStmt(clang::Expr *condExpr, clang::Stmt *thenStmt, clang::Stmt *elseStmt);
  clang::ReturnStmt *createReturnStmt(clang::Expr *expr);
  clang::ImplicitCastExpr *createImplicitCastExpr(clang::Expr *expr, clang::QualType qualType,
                                                  clang::CastKind castKind,
                                                  clang::ExprValueKind valueKind);
  clang::BinaryOperator *createBinaryOperator(clang::BinaryOperator::Opcode opcode,
                                              clang::Expr *lhs, clang::Expr *rhs,
                                              clang::QualType resultType,
                                              clang::ExprValueKind valueKind);
};
