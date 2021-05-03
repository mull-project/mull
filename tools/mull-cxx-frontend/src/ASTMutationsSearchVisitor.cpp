#include "ASTMutationsSearchVisitor.h"

#include "mull/AST/ASTConstants.h"

std::vector<ASTMutation> ASTMutationsSearchVisitor::getAstMutations() {
  return astMutations;
}

bool ASTMutationsSearchVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  llvm::errs() << "visit function: " << FD->getNameInfo().getAsString() << "\n";
  /// FD->dump();
  //    FD->set
  //    FD->removeDecl()
  return true;
}

bool ASTMutationsSearchVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  llvm::errs() << "VisitBinaryOperator: \n";
  binaryOperator->dump();

  for (const std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind> &mutation :
       mull::BINARY_MUTATIONS) {
    if (binaryOperator->getOpcode() == mutation.first && isValidMutation(mutation.second)) {
      clang::SourceLocation binaryOperatorLocation = binaryOperator->getOperatorLoc();
      recordMutationPoint(mutation.second, binaryOperator, binaryOperatorLocation);
    }
  }

  return true;
}

bool ASTMutationsSearchVisitor::isValidMutation(mull::MutatorKind mutatorKind) {
  return mutationsChecklist.count(mutatorKind) > 0;
}

void ASTMutationsSearchVisitor::recordMutationPoint(mull::MutatorKind mutatorKind,
                                                    clang::Stmt *stmt,
                                                    clang::SourceLocation location) {

  int beginLine = sourceManager.getExpansionLineNumber(location, nullptr);
  int beginColumn = sourceManager.getExpansionColumnNumber(location);

  std::string sourceFilePath = sourceManager.getFilename(location).str();
  assert(!sourceFilePath.empty());
  /// if (sourceFilePath.empty()) {
  ///  /// we reach here because of asserts()
  ///  /// TODO: maybe there are more cases.
  ///  return;
  /// }

  ASTMutation astMutation(mutatorKind, stmt, sourceFilePath, beginLine, beginColumn);
  astMutations.emplace_back(astMutation);
}
