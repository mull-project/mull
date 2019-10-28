//
// Created by Stanislav Pankevich on 2019-10-18.
//

#pragma once

#include <clang/AST/Stmt.h>

#include <string>
#include <unordered_map>

namespace mull {
enum TraverseMask : int {
  NONE = 0,

  AND_OR = (1u << 0),
  BINARY_OP = (1u << 1),
  NEGATE = (1u << 3),
  REPLACE_CALL = (1u << 4),
  SCALAR = (1u << 5),
  VOID_CALL = (1u << 6),

  ALL = AND_OR | BINARY_OP | NEGATE | REPLACE_CALL | SCALAR | VOID_CALL
};
struct MutationPointSourceInfo {
  int beginLine = -1;
  int beginColumn = -1;
  int endLine = -1;
  int endColumn = -1;
};

using MutationLocationHash = int;

using SourceFilePath = std::string;

enum ASTNodeType {
  UNKNOWN = 0,

  BINARY_OPERATOR = 1,
  UNARY_OPERATOR,

  RETURN_STATEMENT,

  CALL_EXPRESSION,

  VARIABLE_DECLARATION,

  ARRAY_SUBSCRIPT_EXPRESSION,

  CXX_MEMBER_CALL_EXPRESSION,
  CXX_CONSTRUCTOR_DECLARATION,
  CXX_TEMPORARY_OBJECT_EXPRESSION
};

enum class ASTMutationType { BINARY, AND_OR, SCALAR, REMOVE_VOID, REPLACE_CALL, NEGATE };

struct ASTMutation {
  ASTMutationType mutationType;
  int line;
  int column;
  const clang::Stmt *const stmt;
  ASTMutation(ASTMutationType mutationType, int line, int column,
              const clang::Stmt *const stmt)
      : mutationType(mutationType), line(line), column(column), stmt(stmt) {}
};
using SingleASTUnitMutations =
    std::unordered_map<MutationLocationHash, ASTMutation>;
} // namespace mull
