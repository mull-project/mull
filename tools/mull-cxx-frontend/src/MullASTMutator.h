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
      : _context(context), _factory(context), _instrumentation(context, sema, _factory),
        _clangAstMutator(context, _factory, _instrumentation) {}

  void instrumentTranslationUnit();
  void performBinaryMutation(ASTMutation &mutation, BinaryMutator &binaryMutator) override;
  void performRemoveVoidMutation(ASTMutation &mutation,
                                 RemoveVoidMutator &removeVoidMutator) override;
  void performUnaryOperatorOpcodeReplacementMutation(
      ASTMutation &mutation,
      UnaryOperatorOpcodeReplacementMutator &unaryOperatorOpcodeReplacementMutator) override;
  void
  performUnaryOperatorRemovalMutation(ASTMutation &mutation,
                                      UnaryOperatorRemovalMutator &unaryNotToNoopMutator) override;
  void performReplaceNumericAssignmentMutation(
      ASTMutation &mutation,
      ReplaceNumericAssignmentMutator &replaceNumericAssignmentMutator) override;
  void performReplaceNumericInitAssignmentMutation(
      ASTMutation &mutation,
      ReplaceNumericInitAssignmentMutator &replaceNumericInitAssignmentMutator) override;

private:
  clang::ASTContext &_context;
  ASTNodeFactory _factory;
  ASTInstrumentation _instrumentation;
  ClangASTMutator _clangAstMutator;
};
