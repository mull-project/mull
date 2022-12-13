#include "ASTNodeFactory.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Attr.h>
#include <clang/Basic/Specifiers.h>

namespace mull {
namespace cxx {

const clang::SourceLocation NULL_LOCATION;

clang::FunctionDecl *ASTNodeFactory::createFunctionDecl(std::string name,
                                                        clang::DeclContext *declContext,
                                                        clang::QualType functionType) {
  clang::IdentifierInfo &getenvFuncIdentifierInfo = context.Idents.get(name);
  clang::DeclarationName declarationName(&getenvFuncIdentifierInfo);

  return clang::FunctionDecl::Create(context,
                                     declContext,
                                     NULL_LOCATION,
                                     NULL_LOCATION,
                                     declarationName,
                                     functionType,
                                     context.getTrivialTypeSourceInfo(functionType),
                                     clang::StorageClass::SC_Extern,
#if LLVM_VERSION_MAJOR > 13
                                     false, /// bool UsesFPIntrin = false,
#endif
                                     false, /// bool isInlineSpecified = false,
                                     true,  /// bool hasWrittenPrototype = true,
#if LLVM_VERSION_MAJOR >= 12
                                     clang::ConstexprSpecKind::Unspecified
#else
                                     clang::CSK_unspecified /// ConstexprSpecKind ConstexprKind =
                                                            /// CSK_unspecified
#endif
  );
}

clang::IntegerLiteral *ASTNodeFactory::createIntegerLiteral(int value) {
  return clang::IntegerLiteral::Create(
      context, llvm::APInt(32, value, true), context.IntTy, NULL_LOCATION);
}

clang::FloatingLiteral *ASTNodeFactory::createFloatLiteral(float value) {
  return clang::FloatingLiteral::Create(
      context, llvm::APFloat(value), true, context.FloatTy, NULL_LOCATION);
}

clang::FloatingLiteral *ASTNodeFactory::createFloatLiteral(double value) {
  return clang::FloatingLiteral::Create(
      context, llvm::APFloat(value), true, context.DoubleTy, NULL_LOCATION);
}

clang::StringLiteral *ASTNodeFactory::createStringLiteral(std::string value) {
  return clang::StringLiteral::Create(
      context,
      value,
#if LLVM_VERSION_MAJOR >= 15
      clang::StringLiteral::StringKind::Ordinary,
#else
      clang::StringLiteral::StringKind::Ascii,
#endif
      false,
      getConstantArrayType(context.getConstType(context.CharTy), value.size()),
      NULL_LOCATION);
}

clang::IfStmt *ASTNodeFactory::createIfStmt(clang::Expr *condExpr, clang::Stmt *thenStmt,
                                            clang::Stmt *elseStmt) {
  assert(condExpr);
  assert(thenStmt);
#if LLVM_VERSION_MAJOR >= 12
  clang::IfStmt *ifStmt = clang::IfStmt::Create(context,
                                                NULL_LOCATION,
#if LLVM_VERSION_MAJOR > 13
                                                clang::IfStatementKind::Ordinary,
#else
                                                false,
#endif
                                                nullptr,
                                                nullptr,
                                                condExpr,
                                                NULL_LOCATION,
                                                NULL_LOCATION,
                                                thenStmt,
                                                NULL_LOCATION,
                                                elseStmt);
#else
  clang::IfStmt *ifStmt = clang::IfStmt::Create(
      context, NULL_LOCATION, false, nullptr, nullptr, condExpr, thenStmt, NULL_LOCATION, elseStmt);
#endif
  return ifStmt;
}

clang::ReturnStmt *ASTNodeFactory::createReturnStmt(clang::Expr *expr) {
  return clang::ReturnStmt::Create(context, NULL_LOCATION, expr, NULL);
}

clang::ImplicitCastExpr *ASTNodeFactory::createImplicitCastExpr(clang::Expr *expr,
                                                                clang::QualType qualType,
                                                                clang::CastKind castKind,
                                                                clang::ExprValueKind valueKind) {
  return clang::ImplicitCastExpr::Create(context,
                                         qualType,
                                         castKind,
                                         expr,
                                         nullptr,
                                         valueKind
#if LLVM_VERSION_MAJOR >= 12
                                         ,
                                         clang::FPOptionsOverride()
#endif
  );
}

clang::UnaryOperator *ASTNodeFactory::createUnaryOperator(clang::UnaryOperator::Opcode opcode,
                                                          clang::Expr *expr,
                                                          clang::QualType resultType,
                                                          clang::ExprValueKind valueKind) {
  clang::FPOptionsOverride fpOptionsOverride;
  return clang::UnaryOperator::Create(context,
                                      expr,
                                      opcode,
                                      resultType,
                                      valueKind,
                                      clang::ExprObjectKind::OK_Ordinary,
                                      NULL_LOCATION,
                                      false,
                                      fpOptionsOverride);
}

clang::BinaryOperator *ASTNodeFactory::createBinaryOperator(clang::BinaryOperator::Opcode opcode,
                                                            clang::Expr *lhs, clang::Expr *rhs,
                                                            clang::QualType resultType,
                                                            clang::ExprValueKind valueKind) {
  clang::FPOptionsOverride fpOptionsOverride;
  return clang::BinaryOperator::Create(context,
                                       lhs,
                                       rhs,
                                       opcode,
                                       resultType,
                                       valueKind,
                                       clang::ExprObjectKind::OK_Ordinary,
                                       NULL_LOCATION,
                                       fpOptionsOverride);
}

clang::CompoundAssignOperator *ASTNodeFactory::createCompoundAssignOperator(
    clang::BinaryOperator::Opcode opcode, clang::Expr *lhs, clang::Expr *rhs,
    clang::QualType resultType, clang::ExprValueKind valueKind, clang::QualType compLHSType,
    clang::QualType compResultType) {

  clang::FPOptionsOverride fpOptionsOverride;
  return clang::CompoundAssignOperator::Create(context,
                                               lhs,
                                               rhs,
                                               opcode,
                                               resultType,
                                               valueKind,
                                               clang::ExprObjectKind::OK_Ordinary,
                                               NULL_LOCATION,
                                               fpOptionsOverride,
                                               compLHSType,
                                               compResultType);
}

clang::CallExpr *ASTNodeFactory::createCallExprSingleArg(clang::Expr *function,
                                                         clang::Expr *argument,
                                                         clang::QualType returnType,
                                                         clang::ExprValueKind valueKind) {
  clang::CallExpr *callExpr = clang::CallExpr::Create(context,
                                                      function,
                                                      { argument },
                                                      returnType,
                                                      valueKind,
                                                      NULL_LOCATION
#if LLVM_VERSION_MAJOR >= 12
                                                      ,
                                                      clang::FPOptionsOverride()
#endif
  );

  return callExpr;
}

clang::SectionAttr *ASTNodeFactory::createSectionAttr(std::string sectionName) {
  return clang::SectionAttr::Create(context,
                                    sectionName,
                                    clang::SourceRange(),
                                    clang::AttributeCommonInfo::Syntax::AS_GNU,
                                    clang::SectionAttr::Spelling::SpellingNotCalculated);
}

clang::QualType ASTNodeFactory::getStringLiteralArrayType(clang::QualType type, unsigned size) {
  return context.getStringLiteralArrayType(type, size);
}

clang::QualType ASTNodeFactory::getConstantArrayType(clang::QualType type, unsigned size) {
  return context.getConstantArrayType(
      type, llvm::APInt(8, size + 1), nullptr, clang::ArrayType::ArraySizeModifier::Normal, 0);
}

} // namespace cxx
} // namespace mull
