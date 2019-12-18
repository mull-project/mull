#pragma once

#include "CompilationDatabase.h"

#include <clang/Frontend/ASTUnit.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace mull {

class MutationPoint;
class Diagnostics;

class ThreadSafeASTUnit {
public:
  explicit ThreadSafeASTUnit(clang::ASTUnit *ast);

  clang::SourceManager &getSourceManager();
  clang::ASTContext &getASTContext();

  clang::SourceLocation getLocation(MutationPoint *point);
  bool isInSystemHeader(clang::SourceLocation &location);

  clang::Decl *getDecl(clang::SourceLocation &location);

private:
  void recordDeclarations();

  const clang::FileEntry *findFileEntry(const MutationPoint *point);
  const clang::FileEntry *findFileEntry(const std::string &filePath);

  std::unique_ptr<clang::ASTUnit> ast;
  std::mutex mutex;
  std::vector<clang::Decl *> decls;
};

class ASTStorage {
public:
  ASTStorage(Diagnostics &diagnostics, const std::string &cxxCompilationDatabasePath,
             const std::string &cxxCompilationFlags);

  ThreadSafeASTUnit *findAST(const MutationPoint *point);

  clang::Expr *getMutantASTNode(MutationPoint *mutationPoint);
  void setMutantASTNode(MutationPoint *mutationPoint,
                        clang::Expr *mutantExpression);

private:
  Diagnostics &diagnostics;
  std::mutex mutex;
  std::mutex mutantNodesMutex;

  CompilationDatabase compilationDatabase;
  std::map<std::string, std::unique_ptr<ThreadSafeASTUnit>> astUnits;
  std::map<MutationPoint *, clang::Expr *> mutantASTNodes;
};

} // namespace mull