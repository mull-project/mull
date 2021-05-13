#include "ASTNodeFactory.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Attr.h>

const clang::SourceLocation NULL_LOCATION;

clang::FunctionDecl *ASTNodeFactory::createFunctionDecl(std::string name,
                                                        clang::DeclContext *declContext,
                                                        clang::QualType functionType) {
  clang::IdentifierInfo &getenvFuncIdentifierInfo = _context.Idents.get(name);
  clang::DeclarationName declarationName(&getenvFuncIdentifierInfo);

#if LLVM_VERSION_MAJOR >= 9
  return clang::FunctionDecl::Create(
      _context,
      declContext,
      NULL_LOCATION,
      NULL_LOCATION,
      declarationName,
      functionType,
      _context.getTrivialTypeSourceInfo(functionType),
      clang::StorageClass::SC_Extern,
      false,                 /// bool isInlineSpecified = false,
      true,                  /// bool hasWrittenPrototype = true,
      clang::CSK_unspecified /// ConstexprSpecKind ConstexprKind = CSK_unspecified
  );
#else
  return clang::FunctionDecl::Create(_context,
                                     declContext,
                                     NULL_LOCATION,
                                     NULL_LOCATION,
                                     declarationName,
                                     functionType,
                                     _context.getTrivialTypeSourceInfo(functionType),
                                     clang::StorageClass::SC_Extern,
                                     false, /// bool isInlineSpecified = false,
                                     true,  /// bool hasWrittenPrototype = true,
                                     false  /// bool isConstexprSpecified = false,
  );
#endif
}

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
      getConstantArrayType(_context.getConstType(_context.CharTy), value.size()),
      NULL_LOCATION);
}

clang::IfStmt *ASTNodeFactory::createIfStmt(clang::Expr *condExpr, clang::Stmt *thenStmt,
                                            clang::Stmt *elseStmt) {
  assert(condExpr);
  assert(thenStmt);
#if LLVM_VERSION_MAJOR >= 8
  clang::IfStmt *ifStmt = clang::IfStmt::Create(_context,
                                                NULL_LOCATION,
                                                false,
                                                nullptr,
                                                nullptr,
                                                condExpr,
                                                thenStmt,
                                                NULL_LOCATION,
                                                elseStmt);
#else
  clang::IfStmt *ifStmt = new (_context) clang::IfStmt(_context,
                                                       NULL_LOCATION,
                                                       false,
                                                       nullptr,
                                                       nullptr,
                                                       condExpr,
                                                       thenStmt,
                                                       NULL_LOCATION,
                                                       elseStmt);
#endif
  return ifStmt;
}

clang::ReturnStmt *ASTNodeFactory::createReturnStmt(clang::Expr *expr) {
#if LLVM_VERSION_MAJOR >= 8
  return clang::ReturnStmt::Create(_context, NULL_LOCATION, expr, NULL);
#else
  return new (_context) clang::ReturnStmt(NULL_LOCATION, expr, NULL);
#endif
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
#if LLVM_VERSION_MAJOR >= 11
  clang::FPOptionsOverride fpOptionsOverride;
  return clang::BinaryOperator::Create(_context,
                                       lhs,
                                       rhs,
                                       opcode,
                                       resultType,
                                       valueKind,
                                       clang::ExprObjectKind::OK_Ordinary,
                                       NULL_LOCATION,
                                       fpOptionsOverride);
#else
  clang::FPOptions fpOptions;
  return new (_context) clang::BinaryOperator(lhs,
                                              rhs,
                                              opcode,
                                              resultType,
                                              valueKind,
                                              clang::ExprObjectKind::OK_Ordinary,
                                              NULL_LOCATION,
                                              fpOptions);
#endif
}

clang::CallExpr *ASTNodeFactory::createCallExprSingleArg(clang::Expr *function,
                                                         clang::Expr *argument,
                                                         clang::QualType returnType,
                                                         clang::ExprValueKind valueKind) {
#if LLVM_VERSION_MAJOR >= 8
  clang::CallExpr *callExpr = clang::CallExpr::Create(
      _context, function, { argument }, returnType, valueKind, NULL_LOCATION);
#else
  clang::CallExpr *callExpr = new (_context)
      clang::CallExpr(_context, function, { argument }, returnType, valueKind, NULL_LOCATION);
#endif

  return callExpr;
}

clang::SectionAttr *ASTNodeFactory::createSectionAttr(std::string sectionName) {
#if LLVM_VERSION_MAJOR >= 10
  return clang::SectionAttr::Create(_context,
                                    sectionName,
                                    clang::SourceRange(),
                                    clang::AttributeCommonInfo::Syntax::AS_GNU,
                                    clang::SectionAttr::Spelling::SpellingNotCalculated);
#else
  return new (_context) clang::SectionAttr(clang::SourceRange(), _context, sectionName, 0);
#endif
}

clang::QualType ASTNodeFactory::getStringLiteralArrayType(clang::QualType type, unsigned size) {
#if LLVM_VERSION_MAJOR >= 9
  return _context.getStringLiteralArrayType(type, size);
#else
  return _context.getConstantArrayType(
      type, llvm::APInt(32, size + 1), clang::ArrayType::Normal, /*IndexTypeQuals*/ 0);
#endif
}

clang::QualType ASTNodeFactory::getConstantArrayType(clang::QualType type, unsigned size) {
#if LLVM_VERSION_MAJOR >= 10
  return _context.getConstantArrayType(
      type, llvm::APInt(8, size + 1), nullptr, clang::ArrayType::ArraySizeModifier::Normal, 0);
#else
  return _context.getConstantArrayType(
      type, llvm::APInt(32, size + 1), clang::ArrayType::Normal, /*IndexTypeQuals*/ 0);
#endif
}
