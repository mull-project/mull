#include "mull/AST/ASTMutations.h"

#include <llvm/Support/FileSystem.h>

#include <iostream>

using namespace mull;

ASTMutations::ASTMutations() : storage() {}

int ASTMutations::count() const {
  int count = 0;
  for (auto &pair : storage) {
    /// std::cout << "ASTMutations.count() one ast unit: " << pair.first <<
    /// "\n";
    count += storage.at(pair.first).size();
  }
  return count;
};

bool ASTMutations::locationExists(SourceFilePath sourceFile, int line,
                                  int column) const {
  assert(llvm::sys::fs::is_regular_file(sourceFile) ||
         sourceFile == "input.cc");

  int hash = mull::locationHash(line, column);

  if (storage.count(sourceFile) == 0) {
    return false;
  }

  const SingleASTUnitMutations &astUnitMutations = storage.at(sourceFile);
  return (astUnitMutations.count(hash) != 0);
}

const ASTMutation &ASTMutations::getMutation(const std::string &sourceFile,
                                             int line, int column) const {
  assert(llvm::sys::fs::is_regular_file(sourceFile) ||
         sourceFile == "input.cc");

  assert(storage.count(sourceFile) > 0);

  const SingleASTUnitMutations &astUnitMutations = storage.at(sourceFile);

  MutationLocationHash hash = locationHash(line, column);

  assert(astUnitMutations.count(hash) > 0);

  const ASTMutation &mutation = astUnitMutations.at(hash);
  return mutation;
}

void ASTMutations::dump() const {
  for (auto &entry : storage) {
    std::cout << "file: " << entry.first << "\n";

    for (auto &info : entry.second) {
      std::cout << "line/col: " << info.second.line << " " << info.second.column
                << "\n";
    }
  }
}

void ASTMutations::saveExpr(std::string &sourceFile,
                            ASTMutationType mutationType,
                            ASTNodeType nodeType,
                            const clang::Stmt *const expression, int line,
                            int column) {
  assert(llvm::sys::fs::is_regular_file(sourceFile) ||
         sourceFile == "input.cc");

  int hash = mull::locationHash(line, column);
  storage[sourceFile].emplace(
      hash, ASTMutation(mutationType, line, column, expression));
}
