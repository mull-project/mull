#include "mull/AST/ASTVisitor.h"
#include "mull/AST/MullClangCompatibility.h"

#include "mull/Program/Program.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceLocation.h>

#include <utility>

using namespace mull;

static std::vector<std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind>> BINARY_MUTATIONS = {
  { clang::BO_Add, MutatorKind::CXX_AddToSub },
  { clang::BO_Sub, MutatorKind::CXX_SubToAdd },
  { clang::BO_Mul, MutatorKind::CXX_MulToDiv },
  { clang::BO_Div, MutatorKind::CXX_DivToMul },
  { clang::BO_Rem, MutatorKind::CXX_RemToDiv },

  { clang::BO_AddAssign, MutatorKind::CXX_AddAssignToSubAssign },
  { clang::BO_SubAssign, MutatorKind::CXX_SubAssignToAddAssign },
  { clang::BO_MulAssign, MutatorKind::CXX_MulAssignToDivAssign },
  { clang::BO_DivAssign, MutatorKind::CXX_DivAssignToMulAssign },
  { clang::BO_RemAssign, MutatorKind::CXX_RemAssignToDivAssign },

  { clang::BO_EQ, MutatorKind::CXX_EqualToNotEqual },
  { clang::BO_NE, MutatorKind::CXX_NotEqualToEqual },
  { clang::BO_GT, MutatorKind::CXX_GreaterThanToLessOrEqual },
  { clang::BO_LT, MutatorKind::CXX_LessThanToGreaterOrEqual },
  { clang::BO_GE, MutatorKind::CXX_GreaterOrEqualToLessThan },
  { clang::BO_LE, MutatorKind::CXX_LessOrEqualToGreaterThan },

  { clang::BO_GT, MutatorKind::CXX_GreaterThanToGreaterOrEqual },
  { clang::BO_LT, MutatorKind::CXX_LessThanToLessOrEqual },
  { clang::BO_GE, MutatorKind::CXX_GreaterOrEqualToGreaterThan },
  { clang::BO_LE, MutatorKind::CXX_LessOrEqualToLessThan },

  { clang::BO_And, MutatorKind::CXX_Bitwise_AndToOr },
  { clang::BO_Or, MutatorKind::CXX_Bitwise_OrToAnd },
  { clang::BO_Xor, MutatorKind::CXX_Bitwise_XorToOr },
  { clang::BO_Shl, MutatorKind::CXX_LShiftToRShift },
  { clang::BO_Shr, MutatorKind::CXX_RShiftToLShift },

  { clang::BO_AndAssign, MutatorKind::CXX_Bitwise_AndAssignToOrAssign },
  { clang::BO_OrAssign, MutatorKind::CXX_Bitwise_OrAssignToAndAssign },
  { clang::BO_XorAssign, MutatorKind::CXX_Bitwise_XorAssignToOrAssign },
  { clang::BO_ShlAssign, MutatorKind::CXX_LShiftAssignToRShiftAssign },
  { clang::BO_ShrAssign, MutatorKind::CXX_RShiftAssignToLShiftAssign },

  { clang::BO_LAnd, MutatorKind::CXX_Logical_AndToOr },
  { clang::BO_LOr, MutatorKind::CXX_Logical_OrToAnd }
};

static std::vector<std::pair<clang::UnaryOperator::Opcode, mull::MutatorKind>> UNARY_MUTATIONS = {
  { clang::UO_LNot, MutatorKind::CXX_RemoveNegation },
  { clang::UO_Minus, MutatorKind::CXX_UnaryMinusToNoop }
};

ASTVisitor::ASTVisitor(mull::Diagnostics &diagnostics, mull::ThreadSafeASTUnit &astUnit,
                       mull::SingleASTUnitMutations &singleUnitMutations,
                       mull::FilePathFilter &filePathFilter, mull::MutatorKindSet mutatorKindSet)
    : diagnostics(diagnostics), astUnit(astUnit), singleUnitMutations(singleUnitMutations),
      sourceManager(astUnit.getSourceManager()), filePathFilter(filePathFilter),
      mutatorKindSet(std::move(mutatorKindSet)), scalarMutationMatcher(astUnit.getASTContext()),
      shouldSkipCurrentFunction(false) {}

bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl) {
  if (Decl->getNameAsString() == "main") {
    shouldSkipCurrentFunction = true;
  } else {
    shouldSkipCurrentFunction = false;
  }
  return clang::RecursiveASTVisitor<ASTVisitor>::VisitFunctionDecl(Decl);
}

bool ASTVisitor::VisitExpr(clang::Expr *expr) {
  if (shouldSkipCurrentFunction) {
    return true;
  }

  /// We want to ignore the ImplicitCastExpr nodes because we visit their
  /// children anyway.
  /// Example:
  /// ImplicitCastExpr 0x7fb19806a018 '_Bool' <LValueToRValue>
  /// `-DeclRefExpr 0x7fb198069ff0 '_Bool' lvalue ParmVar 0x7fb198069e38 'a' '_Bool'
  /// There will be another visit on DeclRefExpr.
  if (clang::isa<clang::ImplicitCastExpr>(expr)) {
    return true;
  }

  clang::SourceLocation exprLocation = ClangCompatibilityStmtGetBeginLoc(*expr);
  if (astUnit.isInSystemHeader(exprLocation)) {
    return true;
  }

  const std::string sourceLocation = astUnit.getSourceManager().getFilename(exprLocation).str();
  if (sourceLocation.empty()) {
    /// TODO: asserts only?
    return true;
  }

  if (filePathFilter.shouldSkip(sourceLocation)) {
    return true;
  }

  /// Binary Operators
  if (clang::BinaryOperator *binaryOperator = clang::dyn_cast<clang::BinaryOperator>(expr)) {
    clang::SourceLocation binaryOperatorLocation = binaryOperator->getOperatorLoc();
    for (const std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind> &mutation :
         BINARY_MUTATIONS) {
      if (binaryOperator->getOpcode() == mutation.first &&
          mutatorKindSet.includesMutator(mutation.second)) {
        saveMutationPoint(mutation.second, binaryOperator, binaryOperatorLocation);
      }
    }
    return true;
  }

  /// Remove Void
  if (mutatorKindSet.includesMutator(MutatorKind::RemoveVoidFunctionMutator)) {
    if (const clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(expr)) {
      auto *type = callExpr->getType().getTypePtrOrNull();
      if (type && type->isVoidType()) {
        saveMutationPoint(mull::MutatorKind::RemoveVoidFunctionMutator, callExpr, exprLocation);
      }
      return true;
    }
  }

  /// Replace Call
  if (mutatorKindSet.includesMutator(MutatorKind::ReplaceCallMutator)) {
    if (clang::isa<clang::CallExpr>(expr)) {
      saveMutationPoint(mull::MutatorKind::ReplaceCallMutator, expr, exprLocation);
      return true;
    }
  }

  /// Negate
  if (mutatorKindSet.includesMutator(MutatorKind::NegateMutator)) {
    if (const clang::UnaryOperator *unaryOperator = clang::dyn_cast<clang::UnaryOperator>(expr)) {
      if (unaryOperator->getOpcode() == clang::UnaryOperatorKind::UO_LNot) {
        if (clang::Expr *subExpr = unaryOperator->getSubExpr()) {
          subExpr = subExpr->IgnoreImplicit();

          clang::SourceLocation unaryOperatorBeginLoc =
              ClangCompatibilityStmtGetBeginLoc(*unaryOperator);

          saveMutationPoint(mull::MutatorKind::NegateMutator, unaryOperator, unaryOperatorBeginLoc);
          return true;
        }
      }
      return true;
    }
  }

  /// Unary Operators
  if (auto *unaryOperator = clang::dyn_cast<clang::UnaryOperator>(expr)) {
    clang::SourceLocation location = unaryOperator->getOperatorLoc();
    for (const std::pair<clang::UnaryOperator::Opcode, mull::MutatorKind> &mutation :
         UNARY_MUTATIONS) {
      if (unaryOperator->getOpcode() == mutation.first &&
          mutatorKindSet.includesMutator(mutation.second)) {
        saveMutationPoint(mutation.second, unaryOperator, location);
        return true;
      }
    }
    return true;
  }

  /// Variable declaration
  if (mutatorKindSet.includesMutator(MutatorKind::CXX_InitConst)) {
    clang::SourceLocation mutationLocation;
    bool isMutable = scalarMutationMatcher.isMutableVarDeclExpr(*expr, &mutationLocation);
    if (isMutable) {
      saveMutationPoint(mull::MutatorKind::CXX_InitConst, expr, mutationLocation);
      return true;
    }
  }

  /// Variable assignment
  if (mutatorKindSet.includesMutator(MutatorKind::CXX_AssignConst)) {
    clang::SourceLocation mutationLocation;
    bool isMutable = scalarMutationMatcher.isMutableAssignmentExpr(*expr, &mutationLocation);
    if (isMutable) {
      saveMutationPoint(mull::MutatorKind::CXX_AssignConst, expr, mutationLocation);
      return true;
    }
  }

  /// Scalar
  if (mutatorKindSet.includesMutator(MutatorKind::ScalarValueMutator)) {
    clang::SourceLocation mutationLocation;
    bool isMutable = scalarMutationMatcher.isMutableExpr(*expr, nullptr, &mutationLocation);
    if (isMutable) {
      saveMutationPoint(mull::MutatorKind::ScalarValueMutator, expr, mutationLocation);
      return true;
    }
  }

  return true;
}

void ASTVisitor::traverse() {
  clang::ASTContext &context = astUnit.getASTContext();
  TraverseDecl(context.getTranslationUnitDecl());
}

void ASTVisitor::saveMutationPoint(mull::MutatorKind mutatorKind, const clang::Stmt *stmt,
                                   clang::SourceLocation location) {

  int beginLine = sourceManager.getExpansionLineNumber(location, nullptr);
  int beginColumn = sourceManager.getExpansionColumnNumber(location);

  std::string sourceFilePath = astUnit.getSourceManager().getFilename(location).str();
  if (sourceFilePath.empty()) {
    /// we reach here because of asserts()
    /// TODO: maybe there are more cases.
    return;
  }

  if (!llvm::sys::fs::is_regular_file(sourceFilePath) && sourceFilePath != "/in-memory-file.cc") {
    diagnostics.error(std::string("ASTVisitor: invalid source file path: '") + sourceFilePath);
  }

  std::string description = MutationKindToString(mutatorKind);

  diagnostics.debug(std::string("AST Search: recording mutation \"") + description +
                    "\": " + sourceFilePath + ":" + std::to_string(beginLine) + ":" +
                    std::to_string(beginColumn));

  int hash = mull::lineColumnHash(beginLine, beginColumn);

  if (singleUnitMutations.count(mutatorKind) == 0) {
    singleUnitMutations.emplace(mutatorKind, SingleMutationTypeBucket());
  }

  singleUnitMutations[mutatorKind].emplace(hash,
                                           ASTMutation(mutatorKind, beginLine, beginColumn, stmt));
}
