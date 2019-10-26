//
// Created by Stanislav Pankevich on 2019-10-13.
//

#include "mull/AST/ASTVisitor.h"

#include "mull/Program/Program.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/IR/Module.h>

#include <cassert>

using namespace mull;

static bool isConstant(clang::Stmt *statement) {
  statement = statement->IgnoreImplicit();
  if (clang::isa<clang::IntegerLiteral>(statement)) {
    return true;
  }
  if (clang::isa<clang::FloatingLiteral>(statement)) {
    return true;
  }
  if (clang::isa<clang::CharacterLiteral>(statement)) {
    return true;
  }
  if (const clang::DeclRefExpr *declRefExpr =
          clang::dyn_cast<clang::DeclRefExpr>(statement)) {
    const clang::Decl *referencedDecl =
        declRefExpr->getReferencedDeclOfCallee();
    if (clang::isa<clang::EnumConstantDecl>(referencedDecl)) {
      return true;
    }
  }
  return false;
}

static bool
findPotentialMutableParentStmt(const clang::Stmt *statement,
                               clang::ASTContext &astContext,
                               clang::SourceManager &sourceManager,
                               clang::SourceLocation *mutationLocation) {
  assert(mutationLocation);

  for (auto &parent : astContext.getParents(*statement)) {
    //    llvm::errs() << "\n";
    //    statement->dump(llvm::errs(), sourceManager);
    //    parent.dump(llvm::errs(), sourceManager);
    if (const clang::BinaryOperator *binaryOpParent =
            parent.get<clang::BinaryOperator>()) {
      *mutationLocation = binaryOpParent->getOperatorLoc();
      return true;
    }

    if (const clang::ReturnStmt *returnParent =
            parent.get<clang::ReturnStmt>()) {
      *mutationLocation = returnParent->getBeginLoc();
      return true;
    }

    if (const clang::CXXMemberCallExpr *callExpr =
            parent.get<clang::CXXMemberCallExpr>()) {
      *mutationLocation = callExpr->getExprLoc();
      return true;
    }

    if (const clang::CallExpr *callExpr = parent.get<clang::CallExpr>()) {
      *mutationLocation = callExpr->getBeginLoc();
      return true;
    }

    if (const clang::ImplicitCastExpr *castExpr =
            parent.get<clang::ImplicitCastExpr>()) {
      return findPotentialMutableParentStmt(castExpr, astContext, sourceManager,
                                            mutationLocation);
    }

    if (const clang::CStyleCastExpr *castExpr =
            parent.get<clang::CStyleCastExpr>()) {
      return findPotentialMutableParentStmt(castExpr, astContext, sourceManager,
                                            mutationLocation);
    }

    // TODO: Not implemented
    if (const clang::ConstantExpr *constantExpr =
            parent.get<clang::ConstantExpr>()) {
      return false;
    }

    if (const clang::CXXConstructorDecl *cxxConstructorDecl =
            parent.get<clang::CXXConstructorDecl>()) {

      for (auto &member : cxxConstructorDecl->inits()) {
        if (member->getInit() == statement) {
          *mutationLocation = member->getMemberLocation();
          return true;
        }
      }
    }

    if (const clang::VarDecl *varDecl = parent.get<clang::VarDecl>()) {
      *mutationLocation = varDecl->getLocation();
      return true;
    }

    if (const clang::CXXTemporaryObjectExpr *cxxTemporaryObjectExpr =
            parent.get<clang::CXXTemporaryObjectExpr>()) {
      *mutationLocation = cxxTemporaryObjectExpr->getExprLoc();
      return true;
    }

    // TODO: Not implemented
    llvm::errs() << "\n";
    statement->dump(llvm::errs(), sourceManager);
    parent.dump(llvm::errs(), sourceManager);
    //    assert(0);

    return false;
  }

  return false;
}

ASTVisitor::ASTVisitor(mull::ThreadSafeASTUnit &astUnit,
                       mull::ASTMutations &mutations,
                       mull::FilePathFilter &filePathFilter,
                       mull::TraverseMask traverseMask)
    : astUnit(astUnit), mutations(mutations), filePathFilter(filePathFilter),
      sourceManager(astUnit.getSourceManager()), traverseMask(traverseMask),
      shouldSkipCurrentFunction(false) {}

bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl) {
  if (Decl->getNameAsString() == "main") {
    shouldSkipCurrentFunction = true;
  } else {
    shouldSkipCurrentFunction = false;
  }
  return clang::RecursiveASTVisitor<ASTVisitor>::VisitFunctionDecl(Decl);
}

bool ASTVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  if (shouldSkipCurrentFunction) {
    return true;
  }

  if ((traverseMask & mull::TraverseMask::BINARY_OP) == 0 &&
      (traverseMask & mull::TraverseMask::AND_OR) == 0) {
    return true;
  }

  clang::SourceRange range = binaryOperator->getSourceRange();
  assert(range.isValid());

  clang::SourceLocation beginLocation = binaryOperator->getOperatorLoc();
  beginLocation.dump(sourceManager);

  if (astUnit.isInSystemHeader(beginLocation)) {
    return true;
  }

  saveMutationPoint(binaryOperator, beginLocation);

  return true;
}

bool ASTVisitor::VisitExpr(clang::Expr *expr) {
  if (shouldSkipCurrentFunction) {
    return true;
  }

  clang::SourceLocation exprLocation = expr->getBeginLoc();
  if (astUnit.isInSystemHeader(exprLocation)) {
    return true;
  }

  const std::string sourceLocation =
      astUnit.getSourceManager().getFilename(exprLocation).str();

  if (sourceLocation.empty()) {
    /// TODO: asserts only?
    return true;
  }

  if (filePathFilter.shouldSkip(sourceLocation)) {
    return true;
  }

  /// In case of PredefinedExpr, the expression expr->children() yields
  /// children which are nullptr. These nodes should be junk anyway, so we
  /// ignore them early.
  if (clang::isa<clang::PredefinedExpr>(expr)) {
    return true;
  }

  if ((traverseMask & mull::TraverseMask::NEGATE) != 0) {
    if (clang::isa<clang::ImplicitCastExpr>(expr)) {
      return true;
    }

    if (clang::DeclRefExpr *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
      for (auto &parent : astUnit.getASTContext().getParents(*declRefExpr)) {
        if (const clang::UnaryOperator *unaryOperator = parent.get<clang::UnaryOperator>()) {
          if (clang::Expr *subExpr = unaryOperator->getSubExpr()) {
            saveMutationPoint(unaryOperator, subExpr->getExprLoc());
          }
          return true;
        }
      }

      saveMutationPoint(declRefExpr, declRefExpr->getExprLoc());
    }

    return true;
  }

  if ((traverseMask & mull::TraverseMask::SCALAR) != 0 && isConstant(expr)) {
    expr = expr->IgnoreImplicit();

    //    expr = expr->Ignore();
    /// The constant literals do not have their own debug information in the
    /// LLVM IR if they are part of a parent instruction, such as binary
    /// operator, return instruction, call expr, etc.
    /// Therefore we check for the parent nodes that can result in mutable IR
    /// instructions. We record the parent node' location instead of the
    /// constant location.

    clang::SourceLocation mutationLocation;
    bool foundMutableParent = findPotentialMutableParentStmt(
        expr, astUnit.getASTContext(), sourceManager, &mutationLocation);

    if (foundMutableParent) {
      saveMutationPoint(expr, mutationLocation);
      return true;
    }

    // parent.dump(llvm::errs(), sourceManager);

    /// TODO: STAN
    /// assert(0 && "Should not reach here");
  }

  if (const clang::CallExpr *callExpr =
          clang::dyn_cast<clang::CallExpr>(expr)) {
    if ((traverseMask & mull::TraverseMask::VOID_CALL) != 0) {
      auto *type = callExpr->getType().getTypePtrOrNull();
      if (type && type->isVoidType()) {
        saveMutationPoint(callExpr, callExpr->getBeginLoc());
      }
      return true;
    }
  }

  return true;
}

void ASTVisitor::traverse() {
  clang::ASTContext &context = astUnit.getASTContext();
  TraverseDecl(context.getTranslationUnitDecl());
}

void ASTVisitor::saveMutationPoint(const clang::Stmt *stmt,
                                   clang::SourceLocation location) {

  int beginColumn = sourceManager.getExpansionColumnNumber(location);
  int beginLine = sourceManager.getExpansionLineNumber(location, nullptr);

  std::string sourceFilePath =
      astUnit.getSourceManager().getFilename(location).str();
  if (sourceFilePath.size() == 0) {
    /// we reach here because of asserts()
    /// TODO: maybe there are more cases.
    return;
  }

  if (!llvm::sys::fs::is_regular_file(sourceFilePath) &&
      sourceFilePath != "input.cc") {
    llvm::errs() << "error: ASTVisitor: invalid source file path: '"
                 << sourceFilePath << "'\n";

    exit(1);
  }

  mutations.saveExpr(sourceFilePath, stmt, beginLine, beginColumn);
}
