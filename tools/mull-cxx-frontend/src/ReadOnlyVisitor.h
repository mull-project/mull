//
// Created by Stanislav Pankevich on 23.04.21.
//

#ifndef CLANG_PLUGIN_TEST_SRC_READONLYVISITOR_H
#define CLANG_PLUGIN_TEST_SRC_READONLYVISITOR_H

#include "clang/AST/RecursiveASTVisitor.h"

class ReadOnlyVisitor : public clang::RecursiveASTVisitor<ReadOnlyVisitor> {

public:
  ReadOnlyVisitor() {
  }

  bool VisitImplicitCastExpr(clang::ImplicitCastExpr *implicitCastExpr) {
    return true;
  }

  bool VisitStmt(clang::Stmt *stmt) {
    ///llvm::errs() << "ReadOnlyVisitor::VisitStmt" << "\n";
    ///stmt->dump();
    return true;
  }
};

#endif  // CLANG_PLUGIN_TEST_SRC_READONLYVISITOR_H
