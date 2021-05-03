#pragma once

namespace clang {
class ASTContext;
class FunctionDecl;
} // namespace clang

class ASTNodeFactory;

class ASTInstrumentation {
  clang::ASTContext &_context;
  clang::FunctionDecl *_mullShouldMutateFuncDecl;

public:
  ASTInstrumentation(clang::ASTContext &context)
      : _context(context), _mullShouldMutateFuncDecl(nullptr) {}
  void instrumentTranslationUnit();
  clang::FunctionDecl *getMullShouldMutateFuncDecl();

private:
  clang::FunctionDecl *createMullShouldMutateFuncDecl();
};
