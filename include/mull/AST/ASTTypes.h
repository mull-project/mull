//
// Created by Stanislav Pankevich on 2019-10-18.
//

#pragma once

#include <clang/AST/Stmt.h>

#include <string>
#include <unordered_map>

namespace mull {
enum TraverseMask: int {
  NONE = 0,

  AND_OR = (1u << 0),
  BINARY_OP = (1u << 1),
  NEGATE = (1u << 3),
  SCALAR = (1u << 4),
  VOID_CALL = (1u << 5),

  ALL = AND_OR | BINARY_OP | NEGATE | SCALAR | VOID_CALL
};
struct MutationPointSourceInfo {
  int beginLine = -1;
  int beginColumn = -1;
  int endLine = -1;
  int endColumn = -1;
};

using MutationLocationHash = int;

using SourceFilePath = std::string;

struct ASTMutation {
  int line;
  int column;
  const clang::Stmt *const stmt;
  ASTMutation(int line, int column, const clang::Stmt *const stmt)
    : line(line), column(column), stmt(stmt) {}
};
using SingleASTUnitMutations = std::unordered_map<MutationLocationHash, ASTMutation>;
} // namespace mull
