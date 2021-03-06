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
using SingleFileMutations = std::unordered_map<mull::MutatorKind, SingleMutationTypeBucket>;
/// Single AST Unit consists of mutations found in its .c/.cpp file and mutations
/// found in the header files that are inlined by that AST unit.
using SingleASTUnitMutations = std::unordered_map<SourceFilePath, SingleFileMutations>;

LineColumnHash lineColumnHash(int line, int column);

class ASTMutationStorage {
public:
  std::unordered_map<SourceFilePath, SingleFileMutations> storage;
  Diagnostics &diagnostics;

  ASTMutationStorage(Diagnostics &diagnostics);

  int count() const;

  const ASTMutation &getMutation(const std::string &sourceFile, mull::MutatorKind mutatorKind,
                                 int line, int column) const;

  bool mutationExists(const std::string &sourceFile, mull::MutatorKind mutatorKind, int line,
                      int column) const;

  void saveMutation(const std::string &sourceFile, mull::MutatorKind mutatorKind,
                    const clang::Stmt *const expression, int line, int column);

  void saveMutations(std::unordered_map<SourceFilePath, SingleFileMutations> &storage);
};

} // namespace mull
