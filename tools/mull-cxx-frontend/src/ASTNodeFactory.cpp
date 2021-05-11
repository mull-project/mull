#include "ASTNodeFactory.h"

#include <clang/AST/ASTContext.h>

const clang::SourceLocation NULL_LOCATION;

clang::IntegerLiteral *ASTNodeFactory::createIntegerLiteral(int value) {
  return clang::IntegerLiteral::Create(
      _context, llvm::APInt(32, 0, true), _context.IntTy, NULL_LOCATION);
}

clang::StringLiteral *ASTNodeFactory::createStringLiteral(std::string value) {
  return clang::StringLiteral::Create(
      _context,
      value,
      clang::StringLiteral::StringKind::Ascii,
      false,
      _context.getConstantArrayType(_context.getConstType(_context.CharTy),
                                    llvm::APInt(8, value.size() + 1),
                                    clang::ArrayType::ArraySizeModifier::Normal,
                                    0),
      NULL_LOCATION);
}

clang::IfStmt *ASTNodeFactory::createIfStmt(clang::Expr *condExpr, clang::Stmt *thenStmt,
                                            clang::Stmt *elseStmt) {
  assert(condExpr);
  assert(thenStmt);
  clang::IfStmt *ifStmt = clang::IfStmt::Create(_context,
                                                NULL_LOCATION,
                                                false,
                                                nullptr,
                                                nullptr,
                                                condExpr,
                                                thenStmt,
                                                NULL_LOCATION,
                                                elseStmt);
  return ifStmt;
}

clang::ReturnStmt *ASTNodeFactory::createReturnStmt(clang::Expr *expr) {
  return clang::ReturnStmt::Create(_context, NULL_LOCATION, expr, NULL);
}

clang::ImplicitCastExpr *ASTNodeFactory::createImplicitCastExpr(clang::Expr *expr,
                                                                clang::QualType qualType,
                                                                clang::CastKind castKind,
                                                                clang::ExprValueKind valueKind) {
  return clang::ImplicitCastExpr::Create(_context, qualType, castKind, expr, nullptr, valueKind);
}

clang::BinaryOperator *ASTNodeFactory::createBinaryOperator(clang::BinaryOperator::Opcode opcode,
                                                            clang::Expr *lhs, clang::Expr *rhs,
                                                            clang::QualType resultType,
                                                            clang::ExprValueKind valueKind) {
  clang::FPOptions fpOptions;
  return new (_context) clang::BinaryOperator(lhs,
                                              rhs,
                                              opcode,
                                              resultType,
                                              valueKind,
                                              clang::ExprObjectKind::OK_Ordinary,
                                              NULL_LOCATION,
                                              fpOptions);
}
