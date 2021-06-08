#pragma once

#include <clang/AST/Expr.h>
#include <clang/AST/OperationKinds.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Specifiers.h>

namespace clang {
class SectionAttr;
}

namespace mull {
namespace cxx {

extern const clang::SourceLocation NULL_LOCATION;

class ASTNodeFactory {
  clang::ASTContext &context;

public:
  ASTNodeFactory(clang::ASTContext &context) : context(context) {}

  /// AST Nodes
  clang::FunctionDecl *createFunctionDecl(std::string name, clang::DeclContext *declContext,
                                          clang::QualType functionType);
  clang::IntegerLiteral *createIntegerLiteral(int value);
  clang::FloatingLiteral *createFloatLiteral(float value);
  clang::FloatingLiteral *createFloatLiteral(double value);
  clang::StringLiteral *createStringLiteral(std::string value);
  clang::IfStmt *createIfStmt(clang::Expr *condExpr, clang::Stmt *thenStmt, clang::Stmt *elseStmt);
  clang::ReturnStmt *createReturnStmt(clang::Expr *expr);
  clang::ImplicitCastExpr *createImplicitCastExpr(clang::Expr *expr, clang::QualType qualType,
                                                  clang::CastKind castKind,
                                                  clang::ExprValueKind valueKind);

  clang::UnaryOperator *createUnaryOperator(clang::UnaryOperator::Opcode opcode, clang::Expr *expr,
                                            clang::QualType resultType,
                                            clang::ExprValueKind valueKind);

  clang::BinaryOperator *createBinaryOperator(clang::BinaryOperator::Opcode opcode,
                                              clang::Expr *lhs, clang::Expr *rhs,
                                              clang::QualType resultType,
                                              clang::ExprValueKind valueKind);
  clang::CompoundAssignOperator *createCompoundAssignOperator(clang::BinaryOperator::Opcode opcode,
                                                              clang::Expr *lhs, clang::Expr *rhs,
                                                              clang::QualType resultType,
                                                              clang::ExprValueKind valueKind,
                                                              clang::QualType compLHSType,
                                                              clang::QualType compResultType);

  clang::CallExpr *createCallExprSingleArg(clang::Expr *function, clang::Expr *argument,
                                           clang::QualType returnType,
                                           clang::ExprValueKind valueKind);

  clang::SectionAttr *createSectionAttr(std::string sectionName);

  /// Qual types
  clang::QualType getStringLiteralArrayType(clang::QualType type, unsigned size);
  clang::QualType getConstantArrayType(clang::QualType type, unsigned size);
};

} // namespace cxx
} // namespace mull
