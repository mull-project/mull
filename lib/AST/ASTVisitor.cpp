#include "mull/AST/ASTVisitor.h"

#include "mull/Program/Program.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceLocation.h>

using namespace mull;

static std::vector<std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind>> BINARY_MUTATIONS = {
  { clang::BO_Add, MutatorKind::CXX_AddToSub },
  { clang::BO_Sub, MutatorKind::CXX_SubToAdd },
  { clang::BO_Mul, MutatorKind::CXX_MulToDiv },
  { clang::BO_Div, MutatorKind::CXX_DivToMul },
  { clang::BO_Rem, MutatorKind::CXX_RemToDiv },
  {clang::BO_AddAssign,MutatorKind::CXX_AddAssignToSubAssign},
  {clang::BO_SubAssign,MutatorKind::CXX_SubAssignToAddAssign},
  {clang::BO_MulAssign,MutatorKind::CXX_MulAssignToDivAssign},
};

ASTVisitor::ASTVisitor(mull::Diagnostics &diagnostics, mull::ThreadSafeASTUnit &astUnit,
                       mull::SingleASTUnitMutations &singleUnitMutations,
                       mull::FilePathFilter &filePathFilter, mull::MutatorKindSet mutatorKindSet)
    : diagnostics(diagnostics), astUnit(astUnit), singleUnitMutations(singleUnitMutations),
      filePathFilter(filePathFilter), sourceManager(astUnit.getSourceManager()),
      mutatorKindSet(mutatorKindSet), shouldSkipCurrentFunction(false) {}

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

#if LLVM_VERSION_MAJOR >= 7
  clang::SourceLocation exprLocation = expr->getBeginLoc();
#else
  clang::SourceLocation exprLocation = expr->getLocStart();
#endif

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

  if (clang::BinaryOperator *binaryOperator = clang::dyn_cast<clang::BinaryOperator>(expr)) {
    clang::SourceLocation binaryOperatorLocation = binaryOperator->getOperatorLoc();
    for (const std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind> &mutation :
         BINARY_MUTATIONS) {
      if (binaryOperator->getOpcode() == mutation.first &&
          mutatorKindSet.includesMutator(mutation.second)) {
        saveMutationPoint(mutation.second, binaryOperator, binaryOperatorLocation);
        return true;
      }
    }
    return true;
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

  if (!llvm::sys::fs::is_regular_file(sourceFilePath) && sourceFilePath != "input.cc") {
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
