#include "ASTInstrumentation.h"
#include "ASTNodeFactory.h"

#include <clang/AST/AST.h>
#include <clang/Sema/Sema.h>

#include <sstream>

void ASTInstrumentation::instrumentTranslationUnit() {
  /// Create an external getenv() declaration.
  clang::LinkageSpecDecl *cLinkageSpecDecl =
      clang::LinkageSpecDecl::Create(_context,
                                     _context.getTranslationUnitDecl(),
                                     NULL_LOCATION,
                                     NULL_LOCATION,
                                     clang::LinkageSpecDecl::LanguageIDs::lang_c,
                                     true);

  _getenvFuncDecl = createGetEnvFuncDecl(cLinkageSpecDecl);
  cLinkageSpecDecl->addDecl(_getenvFuncDecl);
  _context.getTranslationUnitDecl()->addDecl(cLinkageSpecDecl);
}

void ASTInstrumentation::addMutantStringDefinition(std::string identifier, int mutator, int line,
                                                   int column) {

  std::ostringstream mis;

  mis << "mull_mutation_" << mutator << "_" << line << "_" << column;
  std::string variableName = mis.str();

  std::string identifierWithCoverage = identifier + ":1";
  clang::IdentifierInfo &varDeclIdentifierInfo = _context.Idents.get(variableName);

  clang::StringLiteral *literal = _factory.createStringLiteral(identifierWithCoverage);
  literal->setValueKind(clang::VK_RValue);

  clang::QualType qualType = _factory.getStringLiteralArrayType(
      _context.getConstType(_context.CharTy), identifierWithCoverage.size());

  clang::VarDecl *varDecl = clang::VarDecl::Create(_context,
                                                   _context.getTranslationUnitDecl(),
                                                   NULL_LOCATION,
                                                   NULL_LOCATION,
                                                   &varDeclIdentifierInfo,
                                                   qualType,
                                                   _context.getTrivialTypeSourceInfo(qualType),
                                                   clang::StorageClass::SC_Extern);
  varDecl->setInit(literal);

#if defined __APPLE__
  const char *mullSection = "__mull,.mull_mutants";
#else
  const char *mullSection = ".mull_mutants";
#endif

  clang::SectionAttr *mullSectionAttr = _factory.createSectionAttr(mullSection);
  varDecl->addAttr(mullSectionAttr);
  _context.getTranslationUnitDecl()->addDecl(varDecl);

  assert(varDecl->isThisDeclarationADefinition() == clang::VarDecl::Definition);

  _sema.WeakTopLevelDecls().push_back(varDecl);
}

clang::FunctionDecl *ASTInstrumentation::getGetenvFuncDecl() {
  assert(_getenvFuncDecl);
  return _getenvFuncDecl;
}

clang::FunctionDecl *ASTInstrumentation::createGetEnvFuncDecl(clang::DeclContext *declContext) {
  /// FunctionDecl 0x7fc7b8897ef0
  /// </Users/Stanislaw/workspace/projects/poc-sandbox/clang-plugin-test/src/fixture.cpp:3:1,
  /// col:33> col:14 getenv 'char *(const char *)' extern
  /// `-ParmVarDecl 0x7fc7b8897e28 <col:21, col:32> col:33 'const char *'

  clang::IdentifierInfo &getenvParamNameInfo = _context.Idents.get("name");

  clang::FunctionProtoType::ExtProtoInfo ext;

  clang::QualType parameterType = _context.getPointerType(_context.getConstType(_context.CharTy));
  clang::QualType returnType = _context.getPointerType(_context.CharTy);

  std::vector<clang::QualType> paramTypes;
  paramTypes.push_back(parameterType);

  clang::QualType getenvFuncType = _context.getFunctionType(returnType, paramTypes, ext);

  clang::FunctionDecl *getEnvFuncDecl =
      _factory.createFunctionDecl("getenv", declContext, getenvFuncType);

  clang::ParmVarDecl *ParDecl =
      clang::ParmVarDecl::Create(_context,
                                 getEnvFuncDecl,
                                 NULL_LOCATION,
                                 NULL_LOCATION,
                                 &getenvParamNameInfo,
                                 parameterType,
                                 _context.getTrivialTypeSourceInfo(parameterType),
                                 clang::StorageClass::SC_None,
                                 nullptr // Expr *DefArg
      );

  std::vector<clang::ParmVarDecl *> paramDecls;
  paramDecls.push_back(ParDecl);
  getEnvFuncDecl->setParams(paramDecls);

  return getEnvFuncDecl;
}
