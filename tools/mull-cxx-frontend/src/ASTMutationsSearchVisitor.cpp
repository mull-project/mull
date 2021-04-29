//
// Created by Stanislav Pankevich on 29.04.21.
//

#include "ASTMutationsSearchVisitor.h"

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

  if (binaryOperator->getOpcode() == clang::BinaryOperator::Opcode::BO_Add) {
    astMutations.emplace_back(ASTMutation::ADD_TO_SUB, binaryOperator);
  } else if (binaryOperator->getOpcode() == clang::BinaryOperator::Opcode::BO_LOr) {
    astMutations.emplace_back(ASTMutation::OR_TO_AND, binaryOperator);
  }

  return true;
}
