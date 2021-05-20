#include "ASTInstrumentation.h"

#include "ClangASTMutator.h"

#include "ASTNodeFactory.h"

#include <clang/AST/ASTContext.h>
#if LLVM_VERSION_MAJOR >= 11
#include <clang/AST/ParentMapContext.h>
#endif
#include <clang/Basic/SourceLocation.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

struct ConditionalOperatorNastyCast : public clang::AbstractConditionalOperator {
  enum { COND, LHS, RHS, END_EXPR };
  Stmt *SubExprs[END_EXPR]; // Left/Middle/Right hand sides.
};
static_assert(sizeof(ConditionalOperatorNastyCast) == sizeof(clang::ConditionalOperator));

void ClangASTMutator::replaceExpression(clang::BinaryOperator *oldBinaryOperator,
                                        clang::BinaryOperator *newBinaryOperator,
                                        std::string identifier) {
  clang::ConditionalOperator *conditionalExpr =
      createMutatedExpression(oldBinaryOperator, newBinaryOperator, identifier);

  for (auto p : _context.getParents(*oldBinaryOperator)) {
    if (const clang::Stmt *constParentStmt = p.get<clang::Stmt>()) {
      clang::Stmt *parentStmt = (clang::Stmt *)constParentStmt;
      clang::Stmt::child_iterator parentChildrenIterator =
          std::find(parentStmt->child_begin(), parentStmt->child_end(), oldBinaryOperator);
      assert(parentChildrenIterator != parentStmt->child_end() && "parent-child relation broken");
      *parentChildrenIterator = conditionalExpr;
    } else if (const clang::VarDecl *constVarDecl = p.get<clang::VarDecl>()) {
      clang::VarDecl *parentVarDecl = (clang::VarDecl *)constVarDecl;
      parentVarDecl->setInit(conditionalExpr);
    } else {
      assert(0 && "error: not implemented");
    }
  }

  //  for (auto p : _context.getParents(*oldBinaryOperator)) {
  //    if (const clang::ReturnStmt *aStmt = p.get<clang::ReturnStmt>()) {
  //      llvm::errs() << "Parent is ReturnStmt\n";
  //      clang::ReturnStmt *returnStmpt = (clang::ReturnStmt *)aStmt;
  //      returnStmpt->setRetValue(conditionalExpr);
  //    } else if (const clang::ParenExpr *constParenExpr = p.get<clang::ParenExpr>()) {
  //      llvm::errs() << "Parent is ParenExpr\n";
  //      clang::ParenExpr *parenExpr = (clang::ParenExpr *)constParenExpr;
  //      parenExpr->setSubExpr(conditionalExpr);
  //    } else if (const clang::IfStmt *constIfStmt = p.get<clang::IfStmt>()) {
  //      llvm::errs() << "Parent is IfStmt\n";
  //      clang::IfStmt *ifStmt = (clang::IfStmt *)constIfStmt;
  //      ifStmt->setCond(conditionalExpr);
  //    } else if (const clang::ConditionalOperator *constConditionalOperator =
  //                   p.get<clang::ConditionalOperator>()) {
  //      llvm::errs() << "Parent is ConditionalOperator\n";
  //      clang::ConditionalOperator *conditionalOperator =
  //          (clang::ConditionalOperator *)constConditionalOperator;
  //      if (oldBinaryOperator == conditionalOperator->getCond()) {
  //        ((ConditionalOperatorNastyCast *)conditionalOperator)
  //            ->SubExprs[ConditionalOperatorNastyCast::COND] = conditionalExpr;
  //      } else if (oldBinaryOperator == conditionalOperator->getLHS()) {
  //        ((ConditionalOperatorNastyCast *)conditionalOperator)
  //            ->SubExprs[ConditionalOperatorNastyCast::LHS] = conditionalExpr;
  //      } else if (oldBinaryOperator == conditionalOperator->getRHS()) {
  //        ((ConditionalOperatorNastyCast *)conditionalOperator)
  //            ->SubExprs[ConditionalOperatorNastyCast::RHS] = conditionalExpr;
  //      } else {
  //        assert(0 && "Should not reach here");
  //      }
  //    } else if (const clang::ImplicitCastExpr *constImplicitCastExpr =
  //                   p.get<clang::ImplicitCastExpr>()) {
  //      llvm::errs() << "Parent is ImplicitCastExpr\n";
  //      clang::ImplicitCastExpr *implicitCastExpr = (clang::ImplicitCastExpr
  //      *)constImplicitCastExpr; implicitCastExpr->setSubExpr(conditionalExpr);
  //    } else if (const clang::VarDecl *constVarDecl = p.get<clang::VarDecl>()) {
  //      llvm::errs() << "Parent is VarDecl\n";
  //      clang::VarDecl *varDecl = (clang::VarDecl *)constVarDecl;
  //      varDecl->setInit(conditionalExpr);
  //    } else if (const clang::BinaryOperator *constBinaryOperator =
  //    p.get<clang::BinaryOperator>()) {
  //      llvm::errs() << "Parent is BinaryOperator\n";
  //      clang::BinaryOperator *binaryOperator = (clang::BinaryOperator *)constBinaryOperator;
  //      if (oldBinaryOperator == binaryOperator->getLHS()) {
  //        binaryOperator->setLHS(conditionalExpr);
  //      } else if (oldBinaryOperator == binaryOperator->getRHS()) {
  //        binaryOperator->setRHS(conditionalExpr);
  //      } else {
  //        assert(0 && "Should not reach here");
  //      }
  //    } else {
  //      p.dump(llvm::errs(), _context.getSourceManager());
  //      assert(0);
  //    }
  //  }
}

void ClangASTMutator::replaceStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                       std::string identifier) {
  clang::IfStmt *ifCondition = createMutatedStatement(oldStmt, newStmt, identifier);

  for (auto p : _context.getParents(*oldStmt)) {
    const clang::Stmt *constParentStmt = p.get<clang::Stmt>();
    clang::Stmt *parentStmt = (clang::Stmt *)constParentStmt;
    assert(parentStmt);

    clang::Stmt::child_iterator parentChildrenIterator =
        std::find(parentStmt->child_begin(), parentStmt->child_end(), oldStmt);
    assert(parentChildrenIterator != parentStmt->child_end() && "parent-child relation broken");

    *parentChildrenIterator = ifCondition;
    return;
  }
  return;
}

clang::IfStmt *ClangASTMutator::createMutatedStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                                       std::string identifier) {
  clang::CallExpr *getenvCallExpr = createGetenvCallExpr(identifier);

  clang::ImplicitCastExpr *implicitCastExpr =
      clang::ImplicitCastExpr::Create(_context,
                                      _context.BoolTy,
                                      clang::CastKind::CK_PointerToBoolean,
                                      getenvCallExpr,
                                      nullptr,
                                      clang::VK_RValue);

  std::vector<clang::Stmt *> thenStmtsVec = {};
  if (newStmt) {
    thenStmtsVec.push_back(newStmt);
  }
  llvm::ArrayRef<clang::Stmt *> thenStmts = thenStmtsVec;
  clang::CompoundStmt *compoundThenStmt =
      clang::CompoundStmt::Create(_context, thenStmts, NULL_LOCATION, NULL_LOCATION);

  llvm::MutableArrayRef<clang::Stmt *> elseStmts = { oldStmt };
  clang::CompoundStmt *compoundElseStmt =
      clang::CompoundStmt::Create(_context, elseStmts, NULL_LOCATION, NULL_LOCATION);

  clang::IfStmt *ifStmt =
      _factory.createIfStmt(implicitCastExpr, compoundThenStmt, compoundElseStmt);

  return ifStmt;
}

clang::ConditionalOperator *ClangASTMutator::createMutatedExpression(clang::Expr *oldExpr,
                                                                     clang::Expr *newExpr,
                                                                     std::string identifier) {
  clang::CallExpr *mullShouldMutateCallExpr = createGetenvCallExpr(identifier);

  clang::ImplicitCastExpr *implicitCastExpr3 =
      clang::ImplicitCastExpr::Create(_context,
                                      _context.BoolTy,
                                      clang::CastKind::CK_PointerToBoolean,
                                      mullShouldMutateCallExpr,
                                      nullptr,
                                      clang::VK_RValue);

  clang::ConditionalOperator *conditionalOperator =
      new (_context) clang::ConditionalOperator(implicitCastExpr3,
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
  clang::FunctionDecl *_getenvFuncDecl = _instrumentation.getGetenvFuncDecl();
  clang::DeclRefExpr *declRefExpr = clang::DeclRefExpr::Create(_context,
                                                               _getenvFuncDecl->getQualifierLoc(),
                                                               NULL_LOCATION,
                                                               _getenvFuncDecl,
                                                               false,
                                                               NULL_LOCATION,
                                                               _getenvFuncDecl->getType(),
                                                               clang::VK_LValue);

  clang::ImplicitCastExpr *implicitCastExpr =
      _factory.createImplicitCastExpr(declRefExpr,
                                      _context.getPointerType(_getenvFuncDecl->getType()),
                                      clang::CastKind::CK_FunctionToPointerDecay,
                                      clang::VK_RValue);

  clang::StringLiteral *stringLiteral = clang::StringLiteral::Create(
      _context,
      identifier,
      clang::StringLiteral::StringKind::Ascii,
      false,
      _factory.getConstantArrayType(_context.CharTy, identifier.size()),
      clang::SourceLocation());

  clang::ImplicitCastExpr *implicitCastExpr2 =
      clang::ImplicitCastExpr::Create(_context,
                                      _context.getPointerType(_context.CharTy),
                                      clang::CastKind::CK_ArrayToPointerDecay,
                                      stringLiteral,
                                      nullptr,
                                      clang::VK_RValue);

  clang::CallExpr *callExpr = _factory.createCallExprSingleArg(
      implicitCastExpr, implicitCastExpr2, _getenvFuncDecl->getReturnType(), clang::VK_RValue);

  return callExpr;
}
