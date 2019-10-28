#pragma once

#include "mull/AST/ASTTypes.h"

#include <clang/AST/Expr.h>

#include <unordered_map>

namespace mull {

/// Cantor pairing function as taken from
/// "Mapping two integers to one, in a unique and deterministic way"
/// https://stackoverflow.com/a/13871379/598057
static MutationLocationHash locationHash(int line, int column) {
  assert(line > 0);
  assert(column > 0);
  return (line + column) * (line + column + 1) / 2 + line;
}

class ASTMutations {
public:
  // TODO: STAN
  std::unordered_map<SourceFilePath, SingleASTUnitMutations> storage;

  ASTMutations();

  int count() const;

  bool locationExists(SourceFilePath sourceFile, int line, int column) const;

  const ASTMutation &getMutation(const std::string &sourceFile, int line,
                                 int column) const;

  void dump() const;

  void saveExpr(std::string &sourceFile, ASTMutationType mutationType,
                ASTNodeType nodeType,
                const clang::Stmt *const expression, int line, int column);
};

} // namespace mull
