#pragma once

#include "mull/AST/ASTMutation.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Mutators/Mutator.h"

#include <clang/AST/Expr.h>

#include <mutex>
#include <string>
#include <unordered_map>

namespace mull {

using LineColumnHash = int;
using SourceFilePath = std::string;

using SingleMutationTypeBucket = std::unordered_map<LineColumnHash, ASTMutation>;
using SingleASTUnitMutations = std::unordered_map<mull::MutatorKind, SingleMutationTypeBucket>;

LineColumnHash lineColumnHash(int line, int column);

class ASTMutationStorage {
public:
  std::unordered_map<SourceFilePath, SingleASTUnitMutations> storage;
  Diagnostics &diagnostics;

  ASTMutationStorage(Diagnostics &diagnostics);

  int count() const;

  const ASTMutation &getMutation(const std::string &sourceFile, mull::MutatorKind mutatorKind,
                                 int line, int column) const;

  bool mutationExists(const std::string &sourceFile, mull::MutatorKind mutatorKind, int line,
                       int column) const;

  void saveMutation(const std::string &sourceFile, mull::MutatorKind mutatorKind,
                    const clang::Stmt *const expression, int line, int column);

  void saveMutations(std::unordered_map<SourceFilePath, SingleASTUnitMutations> &storage);
};

} // namespace mull
