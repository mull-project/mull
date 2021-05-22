#include "ASTMutationsSearchVisitor.h"

#include "MutationMap.h"

#include "mull/AST/ASTConstants.h"

#include "mull/AST/MullClangCompatibility.h"

#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>

std::vector<std::unique_ptr<ASTMutationPoint>> &ASTMutationsSearchVisitor::getAstMutations() {
  return astMutations;
}

bool ASTMutationsSearchVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  /// llvm::errs() << "visit function: " << FD->getNameInfo().getAsString() << "\n";
  /// FD->dump();
  return true;
}

bool ASTMutationsSearchVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_PostDec &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_PostDecToPostInc)) {
    std::unique_ptr<UnaryOperatorOpcodeMutation> mutator =
        std::make_unique<UnaryOperatorOpcodeMutation>(unaryOperator,
                                                      clang::UnaryOperator::Opcode::UO_PostInc);
    recordMutationPoint(mull::MutatorKind::CXX_PostDecToPostInc,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_PostInc &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_PostIncToPostDec)) {
    std::unique_ptr<UnaryOperatorOpcodeMutation> mutator =
        std::make_unique<UnaryOperatorOpcodeMutation>(unaryOperator,
                                                      clang::UnaryOperator::Opcode::UO_PostDec);
    recordMutationPoint(mull::MutatorKind::CXX_PostIncToPostDec,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_PreInc &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_PreIncToPreDec)) {
    std::unique_ptr<UnaryOperatorOpcodeMutation> mutator =
        std::make_unique<UnaryOperatorOpcodeMutation>(unaryOperator,
                                                      clang::UnaryOperator::Opcode::UO_PreDec);
    recordMutationPoint(mull::MutatorKind::CXX_PreIncToPreDec,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_PreDec &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_PreDecToPreInc)) {
    std::unique_ptr<UnaryOperatorOpcodeMutation> mutator =
        std::make_unique<UnaryOperatorOpcodeMutation>(unaryOperator,
                                                      clang::UnaryOperator::Opcode::UO_PreInc);
    recordMutationPoint(mull::MutatorKind::CXX_PreDecToPreInc,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_LNot &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_RemoveNegation)) {
    std::unique_ptr<UnaryOperatorRemovalMutation> mutator =
        std::make_unique<UnaryOperatorRemovalMutation>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_RemoveNegation,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_Minus &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_UnaryMinusToNoop)) {
    std::unique_ptr<UnaryOperatorRemovalMutation> mutator =
        std::make_unique<UnaryOperatorRemovalMutation>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_UnaryMinusToNoop,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_Not &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_BitwiseNotToNoop)) {
    std::unique_ptr<UnaryOperatorRemovalMutation> mutator =
        std::make_unique<UnaryOperatorRemovalMutation>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_BitwiseNotToNoop,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc());
    return true;
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->getOpcode() == clang::BinaryOperator::Opcode::BO_Assign &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_AssignConst)) {
    if (binaryOperator->getRHS()->getType() == _context.IntTy ||
        binaryOperator->getRHS()->getType() == _context.FloatTy ||
        binaryOperator->getRHS()->getType() == _context.DoubleTy) {
      std::unique_ptr<ReplaceNumericAssignmentMutation> mutator =
          std::make_unique<ReplaceNumericAssignmentMutation>(binaryOperator);
      recordMutationPoint(mull::MutatorKind::CXX_AssignConst,
                          std::move(mutator),
                          binaryOperator,
                          binaryOperator->getOperatorLoc());
    }
    return true;
  }

  for (const std::tuple<clang::BinaryOperator::Opcode,
                        mull::MutatorKind,
                        clang::BinaryOperator::Opcode> &mutation : mull::BINARY_MUTATIONS) {
    if (binaryOperator->getOpcode() == std::get<0>(mutation) &&
        mutationMap.isValidMutation(std::get<1>(mutation))) {
      std::unique_ptr<BinaryMutation> binaryMutator =
          std::make_unique<BinaryMutation>(std::get<2>(mutation));
      recordMutationPoint(std::get<1>(mutation),
                          std::move(binaryMutator),
                          binaryOperator,
                          binaryOperator->getOperatorLoc());
    }
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitCallExpr(clang::CallExpr *callExpr) {
  if (callExpr->getType() == _context.VoidTy &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_RemoveVoidCall)) {
    std::unique_ptr<RemoveVoidMutation> removeVoidMutator = std::make_unique<RemoveVoidMutation>();
    recordMutationPoint(mull::MutatorKind::CXX_RemoveVoidCall,
                        std::move(removeVoidMutator),
                        callExpr,
                        ClangCompatibilityStmtGetBeginLoc(*callExpr));
  }

  if (callExpr->getType() == _context.IntTy &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_ReplaceScalarCall)) {
    std::unique_ptr<ReplaceScalarCallMutation> mutator =
        std::make_unique<ReplaceScalarCallMutation>(callExpr);
    recordMutationPoint(mull::MutatorKind::CXX_ReplaceScalarCall,
                        std::move(mutator),
                        callExpr,
                        ClangCompatibilityStmtGetBeginLoc(*callExpr));
  }

  return true;
}

bool ASTMutationsSearchVisitor::VisitVarDecl(clang::VarDecl *D) {
  if (clang::dyn_cast_or_null<clang::ParmVarDecl>(D)) {
    return true;
  }

  if (mutationMap.isValidMutation(mull::MutatorKind::CXX_InitConst) &&
      (D->getType() == _context.IntTy || D->getType() == _context.FloatTy ||
       D->getType() == _context.DoubleTy)) {
    std::unique_ptr<ReplaceNumericInitAssignmentMutation> mutator =
        std::make_unique<ReplaceNumericInitAssignmentMutation>(D);
    recordMutationPoint(mull::MutatorKind::CXX_InitConst,
                        std::move(mutator),
                        D->getInit(),
                        D->getInit()->getExprLoc());
    return true;
  }
  return true;
}

void ASTMutationsSearchVisitor::recordMutationPoint(mull::MutatorKind mutatorKind,
                                                    std::unique_ptr<ASTMutation> mutation,
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

  std::unique_ptr<ASTMutationPoint> astMutation =
      std::make_unique<ASTMutationPoint>(std::move(mutation),
                                         mutatorKind,
                                         mutationMap.getIdentifier(mutatorKind),
                                         stmt,
                                         sourceFilePath,
                                         beginLine,
                                         beginColumn);

  llvm::errs() << "Recording mutation point: " << astMutation->mutationIdentifier << "\n";
  astMutations.emplace_back(std::move(astMutation));
}
