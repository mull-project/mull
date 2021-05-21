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

class ASTInstrumentation;
class ASTNodeFactory;

class ClangASTMutator {
  clang::ASTContext &_context;
  ASTNodeFactory &_factory;
  ASTInstrumentation &_instrumentation;

public:
  ClangASTMutator(clang::ASTContext &context, ASTNodeFactory &factory,
                  ASTInstrumentation &instrumentation)
      : _context(context), _factory(factory), _instrumentation(instrumentation) {}

  void replaceExpression(clang::Expr *oldExpr, clang::Expr *newExpr, std::string identifier);
  void replaceStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt, std::string identifier);
  clang::IfStmt *createMutatedStatement(clang::Stmt *oldStmt, clang::Stmt *newStmt,
                                        std::string identifier);
  clang::ConditionalOperator *createMutatedExpression(clang::Expr *oldExpr, clang::Expr *newExpr,
                                                      std::string identifier);
  clang::CallExpr *createGetenvCallExpr(std::string identifier);
};
