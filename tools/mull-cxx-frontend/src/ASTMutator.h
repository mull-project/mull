//
// Created by Stanislav Pankevich on 26.04.21.
//

#ifndef CLANG_PLUGIN_TEST_SRC_ASTMUTATOR_H
#define CLANG_PLUGIN_TEST_SRC_ASTMUTATOR_H

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

#include "ASTNodeFactory.h"
#include "MullTreeTransform.h"

struct ConditionalOperatorNastyCast : public clang::AbstractConditionalOperator {
  enum { COND, LHS, RHS, END_EXPR };
  Stmt *SubExprs[END_EXPR]; // Left/Middle/Right hand sides.
};
static_assert(sizeof(ConditionalOperatorNastyCast) == sizeof(clang::ConditionalOperator));

class ASTMutator {
  clang::ASTContext &_context;
  ASTNodeFactory &_factory;

  clang::FunctionDecl *_mullShouldMutateFuncDecl;

public:
  ASTMutator(clang::ASTContext &context, ASTNodeFactory &factory,
             clang::FunctionDecl *mullShouldMutateFuncDecl)
      : _context(context), _factory(factory), _mullShouldMutateFuncDecl(mullShouldMutateFuncDecl) {}

  void replaceExpression(clang::BinaryOperator *oldBinaryOperator,
                         clang::BinaryOperator *newBinaryOperator, std::string identifier) {
    clang::ConditionalOperator *conditionalExpr =
        createMutatedExpression(oldBinaryOperator, newBinaryOperator, identifier);

    for (auto p : _context.getParents(*oldBinaryOperator)) {
      if (const clang::ReturnStmt *aStmt = p.get<clang::ReturnStmt>()) {
        llvm::errs() << "Parent is ReturnStmt\n";
        clang::ReturnStmt *returnStmpt = (clang::ReturnStmt *)aStmt;
        returnStmpt->setRetValue(conditionalExpr);
      } else if (const clang::ParenExpr *constParenExpr = p.get<clang::ParenExpr>()) {
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
        if (oldBinaryOperator == conditionalOperator->getCond()) {
          ((ConditionalOperatorNastyCast *)conditionalOperator)
              ->SubExprs[ConditionalOperatorNastyCast::COND] = conditionalExpr;
        } else if (oldBinaryOperator == conditionalOperator->getLHS()) {
          ((ConditionalOperatorNastyCast *)conditionalOperator)
              ->SubExprs[ConditionalOperatorNastyCast::LHS] = conditionalExpr;
        } else if (oldBinaryOperator == conditionalOperator->getRHS()) {
          ((ConditionalOperatorNastyCast *)conditionalOperator)
              ->SubExprs[ConditionalOperatorNastyCast::RHS] = conditionalExpr;
        } else {
          assert(0 && "Should not reach here");
        }
      } else if (const clang::ImplicitCastExpr *constImplicitCastExpr =
                     p.get<clang::ImplicitCastExpr>()) {
        llvm::errs() << "Parent is ImplicitCastExpr\n";
        clang::ImplicitCastExpr *implicitCastExpr =
            (clang::ImplicitCastExpr *)constImplicitCastExpr;
        implicitCastExpr->setSubExpr(conditionalExpr);
      } else if (const clang::VarDecl *constVarDecl = p.get<clang::VarDecl>()) {
        llvm::errs() << "Parent is VarDecl\n";
        clang::VarDecl *varDecl = (clang::VarDecl *)constVarDecl;
        varDecl->setInit(conditionalExpr);
      } else if (const clang::BinaryOperator *constBinaryOperator =
                     p.get<clang::BinaryOperator>()) {
        llvm::errs() << "Parent is BinaryOperator\n";
        clang::BinaryOperator *binaryOperator = (clang::BinaryOperator *)constBinaryOperator;
        if (oldBinaryOperator == binaryOperator->getLHS()) {
          binaryOperator->setLHS(conditionalExpr);
        } else if (oldBinaryOperator == binaryOperator->getRHS()) {
          binaryOperator->setRHS(conditionalExpr);
        } else {
          assert(0 && "Should not reach here");
        }
      } else {
        p.dump(llvm::errs(), _context.getSourceManager());
        assert(0);
      }
    }
  }

  void replaceStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt, std::string identifier) {
    clang::IfStmt *ifCondition = createMutatedStatement(oldStmt, newStmt, identifier);
    ifCondition->dump();

    for (auto p : _context.getParents(*oldStmt)) {
      p.dump(llvm::errs(), _context.getSourceManager());

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

  clang::IfStmt *createMutatedStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                        std::string identifier) {
    clang::CallExpr *mullShouldMutateCallExpr = createMullShouldMutateCallExpr(identifier);

    clang::ImplicitCastExpr *implicitCastExpr =
        clang::ImplicitCastExpr::Create(_context,
                                        _context.BoolTy,
                                        clang::CastKind::CK_PointerToBoolean,
                                        mullShouldMutateCallExpr,
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

    clang::IfStmt *ifStmt = clang::IfStmt::Create(
        _context,
        NULL_LOCATION,
        false,
        nullptr,
        nullptr,
        //                                                    alwaysTrue,
        //                                                    //implicitCastExpr3,
        implicitCastExpr,
        compoundThenStmt,
        NULL_LOCATION,
        compoundElseStmt);

    return ifStmt;
  }

  clang::ConditionalOperator *createMutatedExpression(clang::Expr *oldExpr, clang::Expr *newExpr,
                                                      std::string identifier) {

    //  |   `-ConditionalOperator 0x7f92fb8a5438 <col:11, col:54> 'int'
    //      |     |-ImplicitCastExpr 0x7f92fb8a5420 <col:11, col:46> 'bool' <IntegralToBoolean>
    //      |     | `-CallExpr 0x7f92fb8a53a0 <col:11, col:46> 'int'
    //      |     |   |-ImplicitCastExpr 0x7f92fb8a5388 <col:11> 'int (*)(const char *)'
    //      <FunctionToPointerDecay> |     |   | `-DeclRefExpr 0x7f92fb8a5338 <col:11> 'int (const
    //      char *)' lvalue Function 0x7f92fb833940 'mull_should_mutate_identifier' 'int (const
    //      char*)' |     |   `-ImplicitCastExpr 0x7f92fb8a53c8 <col:41> 'const char *'
    //      <ArrayToPointerDecay> |     |     `-StringLiteral 0x7f92fb8a5318 <col:41> 'const char
    //      [4]' lvalue "FOO" |     |-NEWEXPR |     `-OLDEXPR

    clang::CallExpr *mullShouldMutateCallExpr = createMullShouldMutateCallExpr(identifier);

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

  clang::CallExpr *createMullShouldMutateCallExpr(std::string identifier) {
    assert(_mullShouldMutateFuncDecl);
    // Building CallExpr from the above subtree.
    // clang-format off
    //  |   `-ConditionalOperator 0x7f92fb8a5438 <col:11, col:54> 'int'
    //      |     |-ImplicitCastExpr 0x7f92fb8a5420 <col:11, col:46> 'bool' <IntegralToBoolean>
    //      |     | `-CallExpr 0x7f92fb8a53a0 <col:11, col:46> 'int'
    //      |     |   |-ImplicitCastExpr 0x7f92fb8a5388 <col:11> 'int (*)(const char *)' <FunctionToPointerDecay>
    //      |     |   | `-DeclRefExpr 0x7f92fb8a5338 <col:11> 'int (const char *)' lvalue Function 0x7f92fb833940 'mull_should_mutate_identifier' 'int (const char *)'
    //      |     |   `-ImplicitCastExpr 0x7f92fb8a53c8 <col:41> 'const char *' <ArrayToPointerDecay>
    //      |     |     `-StringLiteral 0x7f92fb8a5318 <col:41> 'const char [4]' lvalue "FOO"
    //      |     |-IntegerLiteral 0x7f92fb8a53e0 <col:50> 'int' 0
    //      |     `-IntegerLiteral 0x7f92fb8a5400 <col:54> 'int' 1
    // clang-format on

    clang::DeclRefExpr *declRefExpr =
        clang::DeclRefExpr::Create(_context,
                                   _mullShouldMutateFuncDecl->getQualifierLoc(),
                                   NULL_LOCATION,
                                   _mullShouldMutateFuncDecl,
                                   false,
                                   NULL_LOCATION,
                                   _mullShouldMutateFuncDecl->getType(),
                                   clang::VK_LValue);

    clang::ImplicitCastExpr *implicitCastExpr = _factory.createImplicitCastExpr(
        declRefExpr,
        _context.getPointerType(_mullShouldMutateFuncDecl->getType()),
        clang::CastKind::CK_FunctionToPointerDecay,
        clang::VK_RValue);

    clang::StringLiteral *stringLiteral = clang::StringLiteral::Create(
        _context,
        identifier,
        clang::StringLiteral::StringKind::Ascii,
        false,
        _context.getConstantArrayType(_context.CharTy,
                                      llvm::APInt(8, identifier.size() + 1),
                                      clang::ArrayType::ArraySizeModifier::Normal,
                                      0),
        clang::SourceLocation());

    clang::ImplicitCastExpr *implicitCastExpr2 =
        clang::ImplicitCastExpr::Create(_context,
                                        _context.getPointerType(_context.CharTy),
                                        clang::CastKind::CK_ArrayToPointerDecay,
                                        stringLiteral,
                                        nullptr,
                                        clang::VK_RValue);

    clang::CallExpr *callExpr = clang::CallExpr::Create(_context,
                                                        implicitCastExpr,
                                                        { implicitCastExpr2 },
                                                        _mullShouldMutateFuncDecl->getReturnType(),
                                                        clang::VK_RValue,
                                                        clang::SourceLocation());
    return callExpr;
  }
};

#endif // CLANG_PLUGIN_TEST_SRC_ASTMUTATOR_H
