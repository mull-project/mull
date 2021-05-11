#pragma once

#include <string>

namespace clang {
class ASTContext;
class DeclContext;
class FunctionDecl;
class Sema;
} // namespace clang

class ASTNodeFactory;

class ASTInstrumentation {
  clang::ASTContext &_context;
  clang::Sema &_sema;
  ASTNodeFactory &_factory;
  clang::FunctionDecl *_getenvFuncDecl;

public:
  ASTInstrumentation(clang::ASTContext &context, clang::Sema &sema, ASTNodeFactory &factory)
      : _context(context), _sema(sema), _factory(factory), _getenvFuncDecl(nullptr) {}
  void instrumentTranslationUnit();
  clang::FunctionDecl *getGetenvFuncDecl();
  void addMutantStringDefinition(std::string identifier, int mutator, int line, int column);

private:
  clang::FunctionDecl *createGetEnvFuncDecl(clang::DeclContext *declContext);
};
