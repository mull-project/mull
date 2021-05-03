#include "ASTInstrumentation.h"
#include "ASTNodeFactory.h"

#include <clang/AST/AST.h>

void ASTInstrumentation::instrumentTranslationUnit() {
  _mullShouldMutateFuncDecl = createMullShouldMutateFuncDecl();
  //  cLinkageSpecDecl->addDecl(_mullShouldMutateFuncDecl);
  _context.getTranslationUnitDecl()->addDecl(_mullShouldMutateFuncDecl);
}

clang::FunctionDecl *ASTInstrumentation::getMullShouldMutateFuncDecl() {
  return _mullShouldMutateFuncDecl;
}

clang::FunctionDecl *ASTInstrumentation::createMullShouldMutateFuncDecl() {
  /// Building this external function declaration:
  ///
  /// extern int mull_should_mutate_identifier(const char *identifier);
  clang::IdentifierInfo &mullShouldMutateFuncIdentifierInfo =
      _context.Idents.get("mull_should_mutate_identifier");
  clang::IdentifierInfo &mullShouldMutateParamIdentifierInfo = _context.Idents.get("identifier");
  clang::DeclarationName declarationName(&mullShouldMutateFuncIdentifierInfo);

  clang::QualType parameterType = _context.getPointerType(_context.getConstType(_context.CharTy));
  clang::QualType returnType = _context.IntTy;

  std::vector<clang::QualType> paramTypes;
  paramTypes.push_back(parameterType);

  clang::QualType mullShouldMutateFuncType =
      _context.getFunctionType(returnType, paramTypes, clang::FunctionProtoType::ExtProtoInfo());

  clang::FunctionDecl *mullShouldMutateFuncDecl = clang::FunctionDecl::Create(
      _context,
      _context.getTranslationUnitDecl(),
      NULL_LOCATION,
      NULL_LOCATION,
      declarationName,
      mullShouldMutateFuncType,
      _context.getTrivialTypeSourceInfo(mullShouldMutateFuncType),
      clang::StorageClass::SC_Extern,
      false,                 /// bool isInlineSpecified = false,
      true,                  /// bool hasWrittenPrototype = true,
      clang::CSK_unspecified /// ConstexprSpecKind ConstexprKind = CSK_unspecified
  );

  clang::ParmVarDecl *ParDecl = clang::ParmVarDecl::Create(_context,
                                                           mullShouldMutateFuncDecl,
                                                           NULL_LOCATION,
                                                           NULL_LOCATION,
                                                           &mullShouldMutateParamIdentifierInfo,
                                                           parameterType,
                                                           _context.getTrivialTypeSourceInfo(parameterType),
                                                           clang::StorageClass::SC_None,
                                                           nullptr // Expr *DefArg
  );
  ParDecl->setIsUsed();
  std::vector<clang::ParmVarDecl *> paramDecls;
  paramDecls.push_back(ParDecl);
  mullShouldMutateFuncDecl->setParams(paramDecls);

  return mullShouldMutateFuncDecl;
}
