#include "ASTMutationsSearchVisitor.h"

#include "mull/AST/ASTConstants.h"

#include "mull/AST/MullClangCompatibility.h"

std::vector<ASTMutation> ASTMutationsSearchVisitor::getAstMutations() {
  return astMutations;
}

bool ASTMutationsSearchVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  /// llvm::errs() << "visit function: " << FD->getNameInfo().getAsString() << "\n";
  /// FD->dump();
  return true;
}

bool ASTMutationsSearchVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  for (const std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind> &mutation :
       mull::BINARY_MUTATIONS) {
    if (binaryOperator->getOpcode() == mutation.first && isValidMutation(mutation.second)) {
      clang::SourceLocation binaryOperatorLocation = binaryOperator->getOperatorLoc();
      recordMutationPoint(mutation.second, binaryOperator, binaryOperatorLocation);
    }
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitCallExpr(clang::CallExpr *callExpr) {
  if (callExpr->getType() == _context.VoidTy) {
    recordMutationPoint(mull::MutatorKind::CXX_RemoveVoidCall, callExpr, ClangCompatibilityStmtGetBeginLoc(*callExpr));
  }
  return true;
}

bool ASTMutationsSearchVisitor::isValidMutation(mull::MutatorKind mutatorKind) {
  return mutationsChecklist.count(mutatorKind) > 0;
}

void ASTMutationsSearchVisitor::recordMutationPoint(mull::MutatorKind mutatorKind,
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
    assert(0);
    return;
  }
  if (sourceFilePath.find("include/gtest") != std::string::npos) {
    return;
  }

  int beginLine = sourceManager.getExpansionLineNumber(location, nullptr);
  int beginColumn = sourceManager.getExpansionColumnNumber(location);

  ASTMutation astMutation(mutatorKind, stmt, sourceFilePath, beginLine, beginColumn);

  llvm::errs() << "Recording mutation point: " << astMutation.mutationIdentifier << "\n";
  astMutations.emplace_back(astMutation);
}
