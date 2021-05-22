#pragma once

#include "ASTInstrumentation.h"
#include "ASTMutationPoint.h"
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
      : _context(context), _factory(context), _instrumentation(context, sema, _factory),
        _clangAstMutator(context, _factory, _instrumentation) {}

  void instrumentTranslationUnit();
  void performBinaryMutation(ASTMutationPoint &mutation, BinaryMutation &binaryMutator) override;
  void performRemoveVoidMutation(ASTMutationPoint &mutation,
                                 RemoveVoidMutation &removeVoidMutator) override;
  void performReplaceScalarMutation(ASTMutationPoint &mutation,
                                    ReplaceScalarCallMutation &replaceScalarCallMutator) override;
  void performUnaryOperatorOpcodeMutation(
      ASTMutationPoint &mutation, UnaryOperatorOpcodeMutation &unaryOperatorOpcodeMutator) override;
  void
  performUnaryOperatorRemovalMutation(ASTMutationPoint &mutation,
                                      UnaryOperatorRemovalMutation &unaryNotToNoopMutator) override;
  void performReplaceNumericAssignmentMutation(
      ASTMutationPoint &mutation,
      ReplaceNumericAssignmentMutation &replaceNumericAssignmentMutator) override;
  void performReplaceNumericInitAssignmentMutation(
      ASTMutationPoint &mutation,
      ReplaceNumericInitAssignmentMutation &replaceNumericInitAssignmentMutator) override;

private:
  clang::ASTContext &_context;
  ASTNodeFactory _factory;
  ASTInstrumentation _instrumentation;
  ClangASTMutator _clangAstMutator;
};
