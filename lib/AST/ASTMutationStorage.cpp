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

ASTMutationStorage::ASTMutationStorage(Diagnostics &diagnostics)
    : storage(), diagnostics(diagnostics) {}

int ASTMutationStorage::count() const {
  int count = 0;
  for (const auto &singleUnitMutations : storage) {
    for (const auto &oneMutationBucket :
         singleUnitMutations.second) {
      count += singleUnitMutations.second.at(oneMutationBucket.first).size();
    }
  }
  return count;
};

const ASTMutation &ASTMutationStorage::getMutation(const std::string &sourceFile,
                                                   MutatorKind mutatorKind, int line,
                                                   int column) const {
  assert(llvm::sys::fs::is_regular_file(sourceFile) || sourceFile == "/in-memory-file.cc");

  assert(storage.count(sourceFile) > 0);

  const SingleASTUnitMutations &astUnitMutations = storage.at(sourceFile);
  assert(astUnitMutations.count(mutatorKind) > 0);

  const SingleMutationTypeBucket &oneMutationBucket = astUnitMutations.at(mutatorKind);

  LineColumnHash hash = lineColumnHash(line, column);
  assert(oneMutationBucket.count(hash) > 0);

  const ASTMutation &mutation = oneMutationBucket.at(hash);
  return mutation;
}

bool ASTMutationStorage::mutationExists(const std::string &sourceFile,
                                        mull::MutatorKind mutatorKind, int line, int column) const {
  assert(llvm::sys::fs::is_regular_file(sourceFile) || sourceFile == "/in-memory-file.cc");

  if (storage.count(sourceFile) == 0) {
    return false;
  }

  const SingleASTUnitMutations &astUnitMutations = storage.at(sourceFile);
  if (astUnitMutations.count(mutatorKind) == 0) {
    return false;
  }

  const SingleMutationTypeBucket &oneMutationBucket = astUnitMutations.at(mutatorKind);

  LineColumnHash hash = lineColumnHash(line, column);
  return oneMutationBucket.count(hash) > 0;
}

void ASTMutationStorage::saveMutation(const std::string &sourceFile, mull::MutatorKind mutatorKind,
                                      const clang::Stmt *const expression, int line, int column) {
  assert(llvm::sys::fs::is_regular_file(sourceFile) || sourceFile == "/in-memory-file.cc");

  std::string description = MutationKindToString(mutatorKind);

  diagnostics.debug(std::string("ASTMutationStorage: recording mutation \"") + description +
                    "\": " + sourceFile + ":" + std::to_string(line) + ":" +
                    std::to_string(column));

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

void ASTMutationStorage::saveMutations(std::unordered_map<SourceFilePath, SingleASTUnitMutations> &mutations) {
  storage.swap(mutations);
}
