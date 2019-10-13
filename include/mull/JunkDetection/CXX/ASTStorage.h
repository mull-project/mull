#pragma once

#include "CompilationDatabase.h"

#include <clang/Frontend/ASTUnit.h>

#include <map>
#include <mutex>
#include <string>

namespace mull {

class MutationPoint;

class ThreadSafeASTUnit {
public:
  explicit ThreadSafeASTUnit(clang::ASTUnit *ast);
  explicit ThreadSafeASTUnit(std::unique_ptr<clang::ASTUnit> ast);

  clang::SourceManager &getSourceManager();
  clang::ASTContext &getASTContext();

  clang::SourceLocation getLocation(MutationPoint *point);
  bool isInSystemHeader(clang::SourceLocation &location);

private:
  const clang::FileEntry *findFileEntry(const MutationPoint *point);
  const clang::FileEntry *findFileEntry(const std::string &filePath);

  std::unique_ptr<clang::ASTUnit> ast;
  std::mutex mutex;
};

class ASTStorage {
public:
  ASTStorage(const std::string &cxxCompilationDatabasePath,
             const std::string &cxxCompilationFlags);

  ThreadSafeASTUnit *findAST(const MutationPoint *point);
  ThreadSafeASTUnit *findASTByPath(const std::string &sourceFile);

  clang::Expr *getMutantASTNode(MutationPoint *mutationPoint);
  void setMutantASTNode(MutationPoint *mutationPoint,
                        clang::Expr *mutantExpression);

private:
  std::mutex mutex;
  std::mutex mutantNodesMutex;

  CompilationDatabase compilationDatabase;
  std::map<std::string, std::unique_ptr<ThreadSafeASTUnit>> astUnits;
  std::map<MutationPoint *, clang::Expr *> mutantASTNodes;
};

} // namespace mull