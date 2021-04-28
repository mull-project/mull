//
// Created by Stanislav Pankevich on 26.04.21.
//

#ifndef CLANG_PLUGIN_TEST_SRC_ASTMUTATOR_H
#define CLANG_PLUGIN_TEST_SRC_ASTMUTATOR_H

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/Support/raw_ostream.h>

#include "MullTreeTransform.h"

struct ConditionalOperatorNastyCast
  : public clang::AbstractConditionalOperator {
  enum { COND, LHS, RHS, END_EXPR };
  Stmt *SubExprs[END_EXPR];  // Left/Middle/Right hand sides.
};

class ASTMutator {
  clang::ASTContext &context;
  MullTreeTransform myTransform;
  clang::FunctionDecl *getenvFuncDecl;

public:
  ASTMutator(clang::ASTContext &context,
             clang::Sema &sema,
             clang::FunctionDecl *getenvFuncDecl)
    : context(context), myTransform(sema), getenvFuncDecl(getenvFuncDecl) {
  }

  void replaceStatement(clang::BinaryOperator *oldBinaryOperator,
                        clang::BinaryOperator *newBinaryOperator) {
    clang::ConditionalOperator *conditionalExpr = createMutatedExpression(
      getenvFuncDecl, oldBinaryOperator, newBinaryOperator);

    for (auto p : context.getParents(*oldBinaryOperator)) {
      if (const clang::ReturnStmt *aStmt = p.get<clang::ReturnStmt>()) {
        llvm::errs() << "Parent is ReturnStmt\n";
        clang::ReturnStmt *returnStmpt = (clang::ReturnStmt *)aStmt;
        returnStmpt->setRetValue(conditionalExpr);
      } else if (const clang::ParenExpr *constParenExpr =
                   p.get<clang::ParenExpr>()) {
        llvm::errs() << "Parent is ParenExpr\n";
        clang::ParenExpr *parenExpr = (clang::ParenExpr *)constParenExpr;
        parenExpr->setSubExpr(conditionalExpr);
      } else if (const clang::IfStmt *constIfStmt = p.get<clang::IfStmt>()) {
        llvm::errs() << "Parent is IfStmt\n";
        clang::IfStmt *ifStmt = (clang::IfStmt *)constIfStmt;
        ifStmt->setCond(conditionalExpr);
      } else if (const clang::ConditionalOperator *constConditionalOperator =
                   p.get<clang::ConditionalOperator>()) {
        llvm::errs() << "Parent is ConditionalOperator\n";
        clang::ConditionalOperator *conditionalOperator =
          (clang::ConditionalOperator *)constConditionalOperator;
        ((ConditionalOperatorNastyCast *)conditionalOperator)
          ->SubExprs[ConditionalOperatorNastyCast::COND] = conditionalExpr;
      } else if (const clang::ImplicitCastExpr *constImplicitCastExpr = p.get<clang::ImplicitCastExpr>()) {
        llvm::errs() << "Parent is ImplicitCastExpr\n";
        clang::ImplicitCastExpr *implicitCastExpr = (clang::ImplicitCastExpr *)constImplicitCastExpr;
        implicitCastExpr->setSubExpr(conditionalExpr);
      }
      else {
        p.dump(llvm::errs(), context.getSourceManager());
        assert(0);
      }
    }
  }

  clang::IfStmt *createMutatedStatement(clang::FunctionDecl *getenvFuncDecl,
                                        clang::Stmt *oldStmt,
                                        clang::Stmt *newStmt) {
    clang::SourceLocation noLoc;  // FD->getBeginLoc();

    clang::DeclRefExpr *declRefExpr =
      clang::DeclRefExpr::Create(context,
                                 getenvFuncDecl->getQualifierLoc(),
                                 noLoc,
                                 getenvFuncDecl,
                                 false,
                                 noLoc,
                                 getenvFuncDecl->getType(),
                                 clang::VK_LValue);

    clang::ImplicitCastExpr *implicitCastExpr = clang::ImplicitCastExpr::Create(
      context,
      context.getPointerType(getenvFuncDecl->getType()),
      clang::CastKind::CK_FunctionToPointerDecay,
      declRefExpr,
      nullptr,
      clang::VK_RValue);

    clang::StringLiteral *stringLiteral = clang::StringLiteral::Create(
      context,
      "FOO",
      clang::StringLiteral::StringKind::Ascii,
      false,
      context.getConstantArrayType(context.CharTy,
                                   llvm::APInt(8, 4),
                                   clang::ArrayType::ArraySizeModifier::Normal,
                                   0),
      noLoc);

    clang::ImplicitCastExpr *implicitCastExpr2 =
      clang::ImplicitCastExpr::Create(context,
                                      context.getPointerType(context.CharTy),
                                      clang::CastKind::CK_ArrayToPointerDecay,
                                      stringLiteral,
                                      nullptr,
                                      clang::VK_RValue);

    clang::CallExpr *calllExpr =
      clang::CallExpr::Create(context,
                              implicitCastExpr,
                              {implicitCastExpr2},
                              getenvFuncDecl->getReturnType(),
                              clang::VK_RValue,
                              noLoc);

    clang::ImplicitCastExpr *implicitCastExpr3 =
      clang::ImplicitCastExpr::Create(context,
                                      context.BoolTy,
                                      clang::CastKind::CK_PointerToBoolean,
                                      calllExpr,
                                      nullptr,
                                      clang::VK_RValue);

    //    clang::ReturnStmt *returnStmt = clang::ReturnStmt::Create(
    //      context,
    //      noLoc,
    //      clang::IntegerLiteral::Create(
    //        context, llvm::APInt(32, 11), context.IntTy, noLoc),
    //      nullptr);
    //
    //    clang::ReturnStmt *returnStmt2 = clang::ReturnStmt::Create(
    //      context,
    //      noLoc,
    //      clang::IntegerLiteral::Create(
    //        context, llvm::APInt(32, 8), context.IntTy, noLoc),
    //      nullptr);
    //
    //    clang::CXXBoolLiteralExpr *alwaysTrue =
    //      new (context) clang::CXXBoolLiteralExpr(false, context.BoolTy,
    //      noLoc);

    clang::IfStmt *ifStmt = clang::IfStmt::Create(
      context,
      noLoc,
      false,
      nullptr,
      nullptr,
      //                                                    alwaysTrue,
      //                                                    //implicitCastExpr3,
      implicitCastExpr3,
      newStmt,
      noLoc,
      oldStmt);

    return ifStmt;
  }

  clang::ConditionalOperator *createMutatedExpression(
    clang::FunctionDecl *getenvFuncDecl,
    clang::Expr *oldExpr,
    clang::Expr *newExpr) {
    clang::SourceLocation noLoc;  // FD->getBeginLoc();

    clang::DeclRefExpr *declRefExpr =
      clang::DeclRefExpr::Create(context,
                                 getenvFuncDecl->getQualifierLoc(),
                                 noLoc,
                                 getenvFuncDecl,
                                 false,
                                 noLoc,
                                 getenvFuncDecl->getType(),
                                 clang::VK_LValue);

    clang::ImplicitCastExpr *implicitCastExpr = clang::ImplicitCastExpr::Create(
      context,
      context.getPointerType(getenvFuncDecl->getType()),
      clang::CastKind::CK_FunctionToPointerDecay,
      declRefExpr,
      nullptr,
      clang::VK_RValue);

    clang::StringLiteral *stringLiteral = clang::StringLiteral::Create(
      context,
      "FOO",
      clang::StringLiteral::StringKind::Ascii,
      false,
      context.getConstantArrayType(context.CharTy,
                                   llvm::APInt(8, 4),
                                   clang::ArrayType::ArraySizeModifier::Normal,
                                   0),
      noLoc);

    clang::ImplicitCastExpr *implicitCastExpr2 =
      clang::ImplicitCastExpr::Create(context,
                                      context.getPointerType(context.CharTy),
                                      clang::CastKind::CK_ArrayToPointerDecay,
                                      stringLiteral,
                                      nullptr,
                                      clang::VK_RValue);

    clang::CallExpr *calllExpr =
      clang::CallExpr::Create(context,
                              implicitCastExpr,
                              {implicitCastExpr2},
                              getenvFuncDecl->getReturnType(),
                              clang::VK_RValue,
                              noLoc);

    clang::ImplicitCastExpr *implicitCastExpr3 =
      clang::ImplicitCastExpr::Create(context,
                                      context.BoolTy,
                                      clang::CastKind::CK_PointerToBoolean,
                                      calllExpr,
                                      nullptr,
                                      clang::VK_RValue);

    //    clang::ReturnStmt *returnStmt = clang::ReturnStmt::Create(
    //      context,
    //      noLoc,
    //      clang::IntegerLiteral::Create(
    //        context, llvm::APInt(32, 11), context.IntTy, noLoc),
    //      nullptr);
    //
    //    clang::ReturnStmt *returnStmt2 = clang::ReturnStmt::Create(
    //      context,
    //      noLoc,
    //      clang::IntegerLiteral::Create(
    //        context, llvm::APInt(32, 8), context.IntTy, noLoc),
    //      nullptr);
    //
    //    clang::CXXBoolLiteralExpr *alwaysTrue =
    //      new (context) clang::CXXBoolLiteralExpr(false, context.BoolTy,
    //      noLoc);

    //    ConditionalOperator(Expr *cond, SourceLocation QLoc, Expr *lhs,
    //      SourceLocation CLoc, Expr *rhs,
    //      QualType t, ExprValueKind VK, ExprObjectKind OK)
    clang::ConditionalOperator *conditionalOperator =
      new (context) clang::ConditionalOperator(implicitCastExpr3,
                                               noLoc,
                                               newExpr,
                                               noLoc,
                                               oldExpr,
                                               newExpr->getType(),
                                               newExpr->getValueKind(),
                                               newExpr->getObjectKind());

    return conditionalOperator;
  }
};

#endif  // CLANG_PLUGIN_TEST_SRC_ASTMUTATOR_H
