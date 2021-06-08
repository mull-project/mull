#pragma once

#include <string>

namespace clang {
class ASTContext;
class DeclContext;
class FunctionDecl;
class Sema;
} // namespace clang

namespace mull {
namespace cxx {

class ASTNodeFactory;

class ASTInstrumentation {
  clang::ASTContext &context;
  clang::Sema &sema;
  ASTNodeFactory &factory;
  clang::FunctionDecl *getenvFuncDecl;

public:
  ASTInstrumentation(clang::ASTContext &context, clang::Sema &sema, ASTNodeFactory &factory)
      : context(context), sema(sema), factory(factory), getenvFuncDecl(nullptr) {}
  void instrumentTranslationUnit();
  clang::FunctionDecl *getGetenvFuncDecl();
  void addMutantStringDefinition(std::string identifier, int mutator, int line, int column);

private:
  clang::FunctionDecl *createGetEnvFuncDecl(clang::DeclContext *declContext);
};

} // namespace cxx
} // namespace mull
