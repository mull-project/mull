#pragma once

#include "ASTInstrumentation.h"
#include "ASTMutation.h"
#include "ASTNodeFactory.h"
#include "ClangASTMutator.h"

namespace clang {
class ASTContext;
class FunctionDecl;
} // namespace clang

class MullASTMutator {
public:
  MullASTMutator(clang::ASTContext &context, clang::Sema &sema)
      : _factory(context), _instrumentation(context, sema, _factory),
        _clangAstMutator(context, _factory, _instrumentation) {}

  void instrumentTranslationUnit();
  void performMutations(std::vector<ASTMutation> &astMutations);

private:
  ASTNodeFactory _factory;
  ASTInstrumentation _instrumentation;
  ClangASTMutator _clangAstMutator;
};
