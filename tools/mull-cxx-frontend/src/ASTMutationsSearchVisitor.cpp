#include "ASTMutationsSearchVisitor.h"

#include "MutationMap.h"

#include "mull/AST/ASTConstants.h"

#include "mull/AST/MullClangCompatibility.h"

#include <clang/Basic/SourceManager.h>

std::vector<std::unique_ptr<ASTMutation>> &ASTMutationsSearchVisitor::getAstMutations() {
  return astMutations;
}

bool ASTMutationsSearchVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  /// llvm::errs() << "visit function: " << FD->getNameInfo().getAsString() << "\n";
  /// FD->dump();
  return true;
}

bool ASTMutationsSearchVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_Not &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_BitwiseNotToNoop)) {
    std::unique_ptr<UnaryNotToNoopMutator> mutator = std::make_unique<UnaryNotToNoopMutator>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_BitwiseNotToNoop,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  for (const std::tuple<clang::BinaryOperator::Opcode,
                        mull::MutatorKind,
                        clang::BinaryOperator::Opcode> &mutation : mull::BINARY_MUTATIONS) {
    if (binaryOperator->getOpcode() == std::get<0>(mutation) &&
        mutationMap.isValidMutation(std::get<1>(mutation))) {
      clang::SourceLocation binaryOperatorLocation = binaryOperator->getOperatorLoc();
      std::unique_ptr<BinaryMutator> binaryMutator =
          std::make_unique<BinaryMutator>(std::get<2>(mutation));
      recordMutationPoint(
          std::get<1>(mutation), std::move(binaryMutator), binaryOperator, binaryOperatorLocation);
    }
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitCallExpr(clang::CallExpr *callExpr) {
  if (callExpr->getType() == _context.VoidTy &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_RemoveVoidCall)) {
    std::unique_ptr<RemoveVoidMutator> removeVoidMutator = std::make_unique<RemoveVoidMutator>();
    recordMutationPoint(mull::MutatorKind::CXX_RemoveVoidCall,
                        std::move(removeVoidMutator),
                        callExpr,
                        ClangCompatibilityStmtGetBeginLoc(*callExpr));
  }
  return true;
}

void ASTMutationsSearchVisitor::recordMutationPoint(mull::MutatorKind mutatorKind,
                                                    std::unique_ptr<Mutator> mutation,
                                                    clang::Stmt *stmt,
                                                    clang::SourceLocation location) {
  if (sourceManager.isInSystemHeader(location)) {
    return;
  }
  if (sourceManager.isInSystemMacro(location)) {
    return;
  }
  std::string sourceFilePath = sourceManager.getFilename(location).str();
  if (sourceFilePath.empty()) {
    /// we reach here because of asserts()
    /// TODO: maybe there are more cases.
    /// assert(0);
    return;
  }
  if (sourceFilePath.find("include/gtest") != std::string::npos) {
    return;
  }

  int beginLine = sourceManager.getExpansionLineNumber(location, nullptr);
  int beginColumn = sourceManager.getExpansionColumnNumber(location);

  std::unique_ptr<ASTMutation> astMutation =
      std::make_unique<ASTMutation>(std::move(mutation),
                                    mutatorKind,
                                    mutationMap.getIdentifier(mutatorKind),
                                    stmt,
                                    sourceFilePath,
                                    beginLine,
                                    beginColumn);

  llvm::errs() << "Recording mutation point: " << astMutation->mutationIdentifier << "\n";
  astMutations.emplace_back(std::move(astMutation));
}
