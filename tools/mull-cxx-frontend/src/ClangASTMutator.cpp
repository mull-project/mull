#include "ASTInstrumentation.h"

#include "ClangASTMutator.h"

#include "ASTNodeFactory.h"

#include <clang/AST/ASTContext.h>
#if LLVM_VERSION_MAJOR >= 11
#include <clang/AST/ParentMapContext.h>
#endif
#include <clang/Basic/SourceLocation.h>
#include <llvm/Support/Casting.h>

namespace mull {
namespace cxx {

void ClangASTMutator::replaceExpression(clang::Expr *oldExpr, clang::Expr *newExpr,
                                        std::string identifier) {
  clang::ConditionalOperator *conditionalExpr =
      createMutatedExpression(oldExpr, newExpr, identifier);

  for (auto p : context.getParents(*oldExpr)) {
    if (const clang::Stmt *constParentStmt = p.get<clang::Stmt>()) {
      /// This is where the actual mutation of expression happens and this where
      /// things play against current Clang AST API.
      /// TODO: Find a better way to perform the mutation.
      clang::Stmt *parentStmt = (clang::Stmt *)constParentStmt;
      clang::Stmt::child_iterator parentChildrenIterator =
          std::find(parentStmt->child_begin(), parentStmt->child_end(), oldExpr);
      assert(parentChildrenIterator != parentStmt->child_end());
      *parentChildrenIterator = conditionalExpr;
      return;
    } else if (const clang::VarDecl *constVarDecl = p.get<clang::VarDecl>()) {
      clang::VarDecl *parentVarDecl = (clang::VarDecl *)constVarDecl;
      parentVarDecl->setInit(conditionalExpr);
      return;
    } else {
      assert(0 && "error: not implemented");
    }
  }
  assert(0 && "Should not reach here");
}

void ClangASTMutator::replaceStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                       std::string identifier) {
  clang::IfStmt *ifCondition = createMutatedStatement(oldStmt, newStmt, identifier);

  for (auto p : context.getParents(*oldStmt)) {
    const clang::Stmt *constParentStmt = p.get<clang::Stmt>();
    clang::Stmt *parentStmt = (clang::Stmt *)constParentStmt;
    assert(parentStmt);

    clang::Stmt::child_iterator parentChildrenIterator =
        std::find(parentStmt->child_begin(), parentStmt->child_end(), oldStmt);
    assert(parentChildrenIterator != parentStmt->child_end());

    *parentChildrenIterator = ifCondition;
    return;
  }
  assert(0 && "Should not reach here");
}

clang::IfStmt *ClangASTMutator::createMutatedStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                                       std::string identifier) {
  clang::CallExpr *getenvCallExpr = createGetenvCallExpr(identifier);

  clang::ImplicitCastExpr *implicitCastExpr =
      clang::ImplicitCastExpr::Create(context,
                                      context.BoolTy,
                                      clang::CastKind::CK_PointerToBoolean,
                                      getenvCallExpr,
                                      nullptr,
#if LLVM_VERSION_MAJOR >= 13
                                      clang::VK_PRValue
#else
                                      clang::VK_RValue
#endif
#if LLVM_VERSION_MAJOR >= 12
                                      ,
                                      clang::FPOptionsOverride()
#endif
      );

  std::vector<clang::Stmt *> thenStmtsVec = {};
  if (newStmt) {
    thenStmtsVec.push_back(newStmt);
  }
  llvm::ArrayRef<clang::Stmt *> thenStmts = thenStmtsVec;
  clang::CompoundStmt *compoundThenStmt =
      clang::CompoundStmt::Create(context, thenStmts, NULL_LOCATION, NULL_LOCATION);

  llvm::MutableArrayRef<clang::Stmt *> elseStmts = { oldStmt };
  clang::CompoundStmt *compoundElseStmt =
      clang::CompoundStmt::Create(context, elseStmts, NULL_LOCATION, NULL_LOCATION);

  clang::IfStmt *ifStmt =
      factory.createIfStmt(implicitCastExpr, compoundThenStmt, compoundElseStmt);

  return ifStmt;
}

clang::ConditionalOperator *ClangASTMutator::createMutatedExpression(clang::Expr *oldExpr,
                                                                     clang::Expr *newExpr,
                                                                     std::string identifier) {
  clang::CallExpr *mullShouldMutateCallExpr = createGetenvCallExpr(identifier);

  clang::ImplicitCastExpr *implicitCastExpr3 =
      clang::ImplicitCastExpr::Create(context,
                                      context.BoolTy,
                                      clang::CastKind::CK_PointerToBoolean,
                                      mullShouldMutateCallExpr,
                                      nullptr,
#if LLVM_VERSION_MAJOR >= 13
                                      clang::VK_PRValue
#else
                                      clang::VK_RValue
#endif
#if LLVM_VERSION_MAJOR >= 12
                                      ,
                                      clang::FPOptionsOverride()
#endif
      );

  clang::ConditionalOperator *conditionalOperator =
      new (context) clang::ConditionalOperator(implicitCastExpr3,
                                               NULL_LOCATION,
                                               newExpr,
                                               NULL_LOCATION,
                                               oldExpr,
                                               newExpr->getType(),
                                               newExpr->getValueKind(),
                                               newExpr->getObjectKind());

  return conditionalOperator;
}

clang::CallExpr *ClangASTMutator::createGetenvCallExpr(std::string identifier) {
  clang::FunctionDecl *_getenvFuncDecl = instrumentation.getGetenvFuncDecl();
  clang::DeclRefExpr *declRefExpr = clang::DeclRefExpr::Create(context,
                                                               _getenvFuncDecl->getQualifierLoc(),
                                                               NULL_LOCATION,
                                                               _getenvFuncDecl,
                                                               false,
                                                               NULL_LOCATION,
                                                               _getenvFuncDecl->getType(),
                                                               clang::VK_LValue);

  clang::ImplicitCastExpr *implicitCastExpr =
      factory.createImplicitCastExpr(declRefExpr,
                                     context.getPointerType(_getenvFuncDecl->getType()),
                                     clang::CastKind::CK_FunctionToPointerDecay,
#if LLVM_VERSION_MAJOR >= 13
                                     clang::VK_PRValue
#else
                                     clang::VK_RValue
#endif
      );

  clang::StringLiteral *stringLiteral = clang::StringLiteral::Create(
      context,
      identifier,
      clang::StringLiteral::StringKind::Ascii,
      false,
      factory.getStringLiteralArrayType(context.CharTy, identifier.size()),
      clang::SourceLocation());

  clang::ImplicitCastExpr *implicitCastExpr2 =
      clang::ImplicitCastExpr::Create(context,
                                      context.getPointerType(context.getConstType(context.CharTy)),
                                      clang::CastKind::CK_ArrayToPointerDecay,
                                      stringLiteral,
                                      nullptr,
#if LLVM_VERSION_MAJOR >= 13
                                      clang::VK_PRValue
#else
                                      clang::VK_RValue
#endif
#if LLVM_VERSION_MAJOR >= 12
                                      ,
                                      clang::FPOptionsOverride()
#endif
      );

  clang::CallExpr *callExpr = factory.createCallExprSingleArg(implicitCastExpr,
                                                              implicitCastExpr2,
                                                              _getenvFuncDecl->getReturnType(),
#if LLVM_VERSION_MAJOR >= 13
                                                              clang::VK_PRValue
#else
                                                              clang::VK_RValue
#endif
  );

  return callExpr;
}

} // namespace cxx
} // namespace mull
