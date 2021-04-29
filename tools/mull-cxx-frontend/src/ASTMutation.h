//
// Created by Stanislav Pankevich on 29.04.21.
//

#ifndef MULL_TOOLS_MULL_CXX_FRONTEND_SRC_ASTMUTATION_H
#define MULL_TOOLS_MULL_CXX_FRONTEND_SRC_ASTMUTATION_H

#include "mull/Mutators/MutatorKind.h"

#include <clang/AST/Stmt.h>

struct ASTMutation {
  mull::MutatorKind mutationType;
  clang::Stmt *mutableStmt;

  ASTMutation(mull::MutatorKind mutationType, clang::Stmt *toBeMutatedStmt)
      : mutationType(mutationType), mutableStmt(toBeMutatedStmt) {}
};

#endif // MULL_TOOLS_MULL_CXX_FRONTEND_SRC_ASTMUTATION_H
