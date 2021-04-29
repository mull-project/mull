//
// Created by Stanislav Pankevich on 29.04.21.
//

#ifndef MULL_TOOLS_MULL_CXX_FRONTEND_SRC_ASTMUTATION_H
#define MULL_TOOLS_MULL_CXX_FRONTEND_SRC_ASTMUTATION_H

#include <clang/AST/AST.h>

struct ASTMutation {
  enum MutationType { ADD_TO_SUB = 0, OR_TO_AND };

  MutationType mutationType;
  clang::Stmt *mutableStmt;

  ASTMutation(MutationType mutationType, clang::Stmt *toBeMutatedStmt)
      : mutationType(mutationType), mutableStmt(toBeMutatedStmt) {}
};

#endif // MULL_TOOLS_MULL_CXX_FRONTEND_SRC_ASTMUTATION_H
