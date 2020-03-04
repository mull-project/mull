#include "mull/AST/ASTMutationStorage.h"

#include <llvm/Support/FileSystem.h>

#include <iostream>

using namespace mull;

/// Cantor pairing function as taken from
/// "Mapping two integers to one, in a unique and deterministic way"
/// https://stackoverflow.com/a/13871379/598057
mull::LineColumnHash mull::lineColumnHash(int line, int column) {
  assert(line > 0);
  assert(column > 0);
  return (line + column) * (line + column + 1) / 2 + line;
}

ASTMutationStorage::ASTMutationStorage() : storage() {}

int ASTMutationStorage::count() const {
  int count = 0;
  for (const std::pair<SourceFilePath, SingleASTUnitMutations> &singleUnitMutations : storage) {
    for (const std::pair<MutatorKind, SingleMutationTypeBucket> &oneMutationBucket :
         singleUnitMutations.second) {
      count += singleUnitMutations.second.at(oneMutationBucket.first).size();
    }
  }
  return count;
};

const ASTMutation &ASTMutationStorage::getMutation(const std::string &sourceFile,
                                                   MutatorKind mutatorKind, int line,
                                                   int column) const {
  assert(llvm::sys::fs::is_regular_file(sourceFile) || sourceFile == "input.cc");

  assert(storage.count(sourceFile) > 0);

  const SingleASTUnitMutations &astUnitMutations = storage.at(sourceFile);
  assert(astUnitMutations.count(mutatorKind) > 0);

  const SingleMutationTypeBucket &oneMutationBucket = astUnitMutations.at(mutatorKind);

  LineColumnHash hash = lineColumnHash(line, column);
  assert(oneMutationBucket.count(hash) > 0);

  const ASTMutation &mutation = oneMutationBucket.at(hash);
  return mutation;
}

void ASTMutationStorage::saveMutation(const std::string &sourceFile, mull::MutatorKind mutatorKind,
                                      const clang::Stmt *const expression, int line, int column) {
  assert(llvm::sys::fs::is_regular_file(sourceFile) || sourceFile == "input.cc");

  int hash = mull::lineColumnHash(line, column);

  if (storage.count(sourceFile) == 0) {
    storage.emplace(sourceFile, SingleASTUnitMutations());
  }

  if (storage[sourceFile].count(mutatorKind) == 0) {
    storage[sourceFile].emplace(mutatorKind, SingleMutationTypeBucket());
  }

  storage[sourceFile][mutatorKind].emplace(hash,
                                           ASTMutation(mutatorKind, line, column, expression));
}
