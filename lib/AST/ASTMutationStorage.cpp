#include "mull/AST/ASTMutationStorage.h"

#include <cassert>
#include <iostream>
#include <llvm/Support/FileSystem.h>

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
  for (const auto &singleFileMutations : storage) {
    for (const auto &oneMutationBucket : singleFileMutations.second) {
      count += singleFileMutations.second.at(oneMutationBucket.first).size();
    }
  }
  return count;
};

const ASTMutation &ASTMutationStorage::getMutation(const std::string &sourceFile,
                                                   MutatorKind mutatorKind, int line,
                                                   int column) const {
  assert(llvm::sys::fs::is_regular_file(sourceFile) || sourceFile == "/in-memory-file.cc");

  assert(storage.count(sourceFile) > 0);

  const SingleFileMutations &singleFileMutations = storage.at(sourceFile);
  assert(singleFileMutations.count(mutatorKind) > 0);

  const SingleMutationTypeBucket &oneMutationBucket = singleFileMutations.at(mutatorKind);

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

  const SingleFileMutations &singleFileMutations = storage.at(sourceFile);
  if (singleFileMutations.count(mutatorKind) == 0) {
    return false;
  }

  const SingleMutationTypeBucket &oneMutationBucket = singleFileMutations.at(mutatorKind);

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
    storage.emplace(sourceFile, SingleFileMutations());
  }

  if (storage[sourceFile].count(mutatorKind) == 0) {
    storage[sourceFile].emplace(mutatorKind, SingleMutationTypeBucket());
  }

  storage[sourceFile][mutatorKind].emplace(hash,
                                           ASTMutation(mutatorKind, line, column, expression));
}

void ASTMutationStorage::saveMutations(
    std::unordered_map<SourceFilePath, SingleFileMutations> &mutations) {
  storage.swap(mutations);
}
