#include "ASTMutationsSearchVisitor.h"

#include "MutationMap.h"

#include "mull/AST/ASTConstants.h"
#include "mull/AST/MullClangCompatibility.h"

#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

std::vector<std::unique_ptr<ASTMutationPoint>> &ASTMutationsSearchVisitor::getAstMutations() {
  return astMutations;
}

bool ASTMutationsSearchVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  /// No-op. Useful for debugging.
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
                        unaryOperator->getOperatorLoc(),
                        false);
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
                        unaryOperator->getOperatorLoc(),
                        false);
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
                        unaryOperator->getOperatorLoc(),
                        false);
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
                        unaryOperator->getOperatorLoc(),
                        false);
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_LNot &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_RemoveNegation)) {
    std::unique_ptr<UnaryOperatorRemovalMutation> mutator =
        std::make_unique<UnaryOperatorRemovalMutation>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_RemoveNegation,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc(),
                        false);
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_Minus &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_UnaryMinusToNoop)) {
    std::unique_ptr<UnaryOperatorRemovalMutation> mutator =
        std::make_unique<UnaryOperatorRemovalMutation>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_UnaryMinusToNoop,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc(),
                        false);
    return true;
  }

  if (unaryOperator->getOpcode() == clang::UnaryOperator::Opcode::UO_Not &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_BitwiseNotToNoop)) {
    std::unique_ptr<UnaryOperatorRemovalMutation> mutator =
        std::make_unique<UnaryOperatorRemovalMutation>(unaryOperator);
    recordMutationPoint(mull::MutatorKind::CXX_BitwiseNotToNoop,
                        std::move(mutator),
                        unaryOperator,
                        unaryOperator->getOperatorLoc(),
                        false);
    return true;
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->getOpcode() == clang::BinaryOperator::Opcode::BO_Assign &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_AssignConst)) {
    if (binaryOperator->getRHS()->getType() == context.IntTy ||
        binaryOperator->getRHS()->getType() == context.FloatTy ||
        binaryOperator->getRHS()->getType() == context.DoubleTy) {
      std::unique_ptr<ReplaceNumericAssignmentMutation> mutator =
          std::make_unique<ReplaceNumericAssignmentMutation>(binaryOperator);
      recordMutationPoint(mull::MutatorKind::CXX_AssignConst,
                          std::move(mutator),
                          binaryOperator,
                          binaryOperator->getOperatorLoc(),
                          true);
    }
    return true;
  }

  for (const std::tuple<clang::BinaryOperator::Opcode,
                        mull::MutatorKind,
                        clang::BinaryOperator::Opcode> &mutation : mull::BinaryMutations) {
    if (binaryOperator->getOpcode() == std::get<0>(mutation) &&
        mutationMap.isValidMutation(std::get<1>(mutation))) {
      std::unique_ptr<BinaryMutation> binaryMutator =
          std::make_unique<BinaryMutation>(std::get<2>(mutation));
      recordMutationPoint(std::get<1>(mutation),
                          std::move(binaryMutator),
                          binaryOperator,
                          binaryOperator->getOperatorLoc(),
                          false);
    }
  }
  return true;
}

bool ASTMutationsSearchVisitor::VisitCallExpr(clang::CallExpr *callExpr) {
  if (callExpr->getType() == context.VoidTy &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_RemoveVoidCall)) {
    std::unique_ptr<RemoveVoidMutation> removeVoidMutator = std::make_unique<RemoveVoidMutation>();
    recordMutationPoint(mull::MutatorKind::CXX_RemoveVoidCall,
                        std::move(removeVoidMutator),
                        callExpr,
                        ClangCompatibilityStmtGetBeginLoc(*callExpr),
                        true);
  }

  if (callExpr->getType() == context.IntTy &&
      mutationMap.isValidMutation(mull::MutatorKind::CXX_ReplaceScalarCall)) {
    std::unique_ptr<ReplaceScalarCallMutation> mutator =
        std::make_unique<ReplaceScalarCallMutation>(callExpr);
    recordMutationPoint(mull::MutatorKind::CXX_ReplaceScalarCall,
                        std::move(mutator),
                        callExpr,
                        ClangCompatibilityStmtGetBeginLoc(*callExpr),
                        true);
  }

  return true;
}

bool ASTMutationsSearchVisitor::VisitVarDecl(clang::VarDecl *D) {
  /// Function parameter declarations are not mutated.
  if (clang::dyn_cast_or_null<clang::ParmVarDecl>(D)) {
    return true;
  }

  if (mutationMap.isValidMutation(mull::MutatorKind::CXX_InitConst) &&
      (D->getType() == context.IntTy || D->getType() == context.FloatTy ||
       D->getType() == context.DoubleTy)) {
    std::unique_ptr<ReplaceNumericInitAssignmentMutation> mutator =
        std::make_unique<ReplaceNumericInitAssignmentMutation>(D);
    recordMutationPoint(mull::MutatorKind::CXX_InitConst,
                        std::move(mutator),
                        D->getInit(),
                        D->getInit()->getExprLoc(),
                        true);
    return true;
  }
  return true;
}

void ASTMutationsSearchVisitor::recordMutationPoint(mull::MutatorKind mutatorKind,
                                                    std::unique_ptr<ASTMutation> mutation,
                                                    clang::Stmt *stmt,
                                                    clang::SourceLocation beginLocation,
                                                    bool locationIsExpression) {
  if (sourceManager.isInSystemHeader(beginLocation)) {
    return;
  }
  if (sourceManager.isInSystemMacro(beginLocation)) {
    return;
  }
  std::string sourceFilePath = sourceManager.getFilename(beginLocation).str();
  if (sourceFilePath.empty()) {
    /// we reach here because of asserts()
    /// TODO: maybe there are more cases.
    /// assert(0);
    return;
  }
  if (sourceFilePath.find("include/gtest") != std::string::npos) {
    return;
  }

  clang::SourceLocation endLocation =
      locationIsExpression ? stmt->getSourceRange().getEnd() : beginLocation;

  /// Clang AST: how to get more precise debug information in certain cases?
  /// http://clang-developers.42468.n3.nabble.com/Clang-AST-how-to-get-more-precise-debug-information-in-certain-cases-td4065195.html
  /// https://stackoverflow.com/questions/11083066/getting-the-source-behind-clangs-ast
  clang::SourceLocation sourceLocationEndActual =
      clang::Lexer::getLocForEndOfToken(endLocation, 0, sourceManager, context.getLangOpts());

  int beginLine = sourceManager.getExpansionLineNumber(beginLocation, nullptr);
  int beginColumn = sourceManager.getExpansionColumnNumber(beginLocation);

  int endLine = sourceManager.getExpansionLineNumber(sourceLocationEndActual, nullptr);
  int endColumn = sourceManager.getExpansionColumnNumber(sourceLocationEndActual);

  std::unique_ptr<ASTMutationPoint> astMutation =
      std::make_unique<ASTMutationPoint>(std::move(mutation),
                                         mutatorKind,
                                         mutationMap.getIdentifier(mutatorKind),
                                         stmt,
                                         sourceFilePath,
                                         beginLine,
                                         beginColumn,
                                         endLine,
                                         endColumn);

  llvm::outs() << "Recording mutation point: " << astMutation->mutationIdentifier << " (end: "
               << std::to_string(endLine) << ":" << std::to_string(endColumn) << ")\n";
  astMutations.emplace_back(std::move(astMutation));
}
