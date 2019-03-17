#pragma once

#include "JunkDetection/CXX/CompilationDatabase.h"

#include <clang/Frontend/ASTUnit.h>

#include <map>
#include <mutex>
#include <string>

namespace mull {

class MutationPoint;

class ThreadSafeASTUnit {
public:
  explicit ThreadSafeASTUnit(clang::ASTUnit *ast);

  clang::SourceManager &getSourceManager();
  clang::ASTContext &getASTContext();

  clang::SourceLocation getLocation(MutationPoint *point);
  bool isInSystemHeader(clang::SourceLocation &location);

private:
  const clang::FileEntry *findFileEntry(const MutationPoint *point);

  std::unique_ptr<clang::ASTUnit> ast;
  std::mutex mutex;
};

class ASTStorage {
public:
  ASTStorage(const std::string &cxxCompilationDatabasePath,
             const std::string &cxxCompilationFlags);

  ThreadSafeASTUnit *findAST(const MutationPoint *point);

private:
  std::mutex mutex;

  CompilationDatabase compilationDatabase;
  std::map<std::string, std::unique_ptr<ThreadSafeASTUnit>> astUnits;
};

} // namespace mull