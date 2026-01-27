#pragma once

#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include "mull/SourceLocation.h"

#include <clang/Frontend/ASTUnit.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mull {

class MutationPoint;
class Diagnostics;

class ASTUnitWrapper {
public:
  explicit ASTUnitWrapper(std::unique_ptr<clang::ASTUnit> ast);

  clang::SourceManager &getSourceManager();
  clang::ASTContext &getASTContext();

  clang::SourceLocation getLocation(const mull::SourceLocation &sourceLocation);
  clang::SourceLocation getLocForEndOfToken(const clang::SourceLocation sourceLocationEnd);
  bool isInSystemHeader(clang::SourceLocation &location);

  clang::Decl *getDecl(clang::SourceLocation &location);
  bool hasAST() const;

private:
  void recordDeclarations();

  const clang::FileEntry *findFileEntry(const mull::SourceLocation &sourceLocation);
  const clang::FileEntry *findFileEntry(const std::string &filePath);

  std::unique_ptr<clang::ASTUnit> ast;
  std::vector<clang::Decl *> decls;
};

class ASTStorage {
public:
  ASTStorage(Diagnostics &diagnostics, const std::string &cxxCompilationDatabasePath,
             const std::string &cxxCompilationFlags,
             const std::unordered_map<std::string, std::string> &bitcodeCompilationFlags);

  ASTUnitWrapper *findAST(const mull::SourceLocation &sourceLocation);
  ASTUnitWrapper *findAST(const std::string &sourceFile);

  void setAST(const std::string &sourceFile, std::unique_ptr<ASTUnitWrapper> astUnit);

private:
  Diagnostics &diagnostics;
  CompilationDatabase compilationDatabase;
  std::unordered_map<std::string, std::unique_ptr<ASTUnitWrapper>> astUnits;
};

} // namespace mull
