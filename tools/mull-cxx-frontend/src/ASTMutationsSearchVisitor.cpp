#include "ASTMutationsSearchVisitor.h"

#include "mull/AST/ASTConstants.h"

std::vector<ASTMutation> ASTMutationsSearchVisitor::getAstMutations() {
  return astMutations;
}

bool ASTMutationsSearchVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  llvm::errs() << "visit function: " << FD->getNameInfo().getAsString() << "\n";
  FD->dump();
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
      astMutations.emplace_back(mutation.second, binaryOperator);
    }
  }

  return true;
}

bool ASTMutationsSearchVisitor::isValidMutation(mull::MutatorKind mutatorKind) {
  return mutationsChecklist.count(mutatorKind) > 0;
}
