#pragma once

#include <string>

namespace clang {
class ASTContext;
class BinaryOperator;
class CallExpr;
class ConditionalOperator;
class FunctionDecl;
class Expr;
class IfStmt;
class Stmt;
} // namespace clang

class ASTNodeFactory;

class ClangASTMutator {
  clang::ASTContext &_context;
  ASTNodeFactory &_factory;

  clang::FunctionDecl *_getenvFuncDecl;

public:
  ClangASTMutator(clang::ASTContext &context, ASTNodeFactory &factory,
                  clang::FunctionDecl *getenvFuncDecl)
      : _context(context), _factory(factory), _getenvFuncDecl(getenvFuncDecl) {}

  void replaceExpression(clang::BinaryOperator *oldBinaryOperator,
                         clang::BinaryOperator *newBinaryOperator, std::string identifier);
  void replaceStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt, std::string identifier);
  clang::IfStmt *createMutatedStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                        std::string identifier);
  clang::ConditionalOperator *createMutatedExpression(clang::Expr *oldExpr, clang::Expr *newExpr,
                                                      std::string identifier);
  clang::CallExpr *createGetenvCallExpr(std::string identifier);
};
