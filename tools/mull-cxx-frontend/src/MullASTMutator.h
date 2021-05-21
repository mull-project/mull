#pragma once

#include "ASTInstrumentation.h"
#include "ASTMutation.h"
#include "ASTMutator.h"
#include "ASTNodeFactory.h"
#include "ClangASTMutator.h"

namespace clang {
class ASTContext;
class FunctionDecl;
} // namespace clang

class MullASTMutator : public ASTMutator {
public:
  MullASTMutator(clang::ASTContext &context, clang::Sema &sema)
      : _factory(context), _instrumentation(context, sema, _factory),
        _clangAstMutator(context, _factory, _instrumentation) {}

  void instrumentTranslationUnit();
  void performBinaryMutation(ASTMutation &mutation, BinaryMutator &binaryMutator) override;
  void performRemoveVoidMutation(ASTMutation &mutation,
                                 RemoveVoidMutator &removeVoidMutator) override;
  void performUnaryNotToNoopMutator(ASTMutation &mutation,
                                    UnaryNotToNoopMutator &unaryNotToNoopMutator) override;

private:
  ASTNodeFactory _factory;
  ASTInstrumentation _instrumentation;
  ClangASTMutator _clangAstMutator;
};
