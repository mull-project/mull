#pragma once

#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include "mull/SourceLocation.h"

#include <clang/Frontend/ASTUnit.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct MullDiagnostics;

namespace mull {

class MutationPoint;

class ASTUnitWrapper {
public:
  explicit ASTUnitWrapper(std::unique_ptr<clang::ASTUnit> ast);

  clang::SourceManager &getSourceManager();
  clang::ASTContext &getASTContext();

  clang::SourceLocation getLocation(const mull::SourceLocation &sourceLocation);
  clang::CharSourceRange getFileRange(clang::SourceLocation begin, clang::SourceLocation end);
  bool isInSystemHeader(clang::SourceLocation &location);

  clang::Decl *getDecl(clang::SourceLocation &location);
  /// The definition, e.g. a template instantiation, emitted under the mangled name
  clang::FunctionDecl *getFunction(const std::string &mangledName);
  bool hasAST() const;

private:
  void recordDeclarations();
  void recordFunctions();
  void recordFileEntries();

  clang::FileID findFileID(const mull::SourceLocation &sourceLocation);
  clang::FileID findFileID(const std::string &filePath);

  std::unique_ptr<clang::ASTUnit> ast;
  std::vector<clang::Decl *> decls;
  /// Built on first use
  std::optional<std::unordered_map<std::string, clang::FunctionDecl *>> functionsByName;
#if LLVM_VERSION_MAJOR >= 18
  using FileRef = clang::FileEntryRef;
#else
  using FileRef = const clang::FileEntry *;
#endif
  /// Real path -> file the source manager knows. Translating it to a FileID can load source
  /// locations of a PCH, which adds files to the table this is recorded from.
  std::unordered_map<std::string, FileRef> filesByRealPath;
  size_t recordedFileCount = 0;
  /// Debug info path -> file, invalid if not part of the translation unit
  std::unordered_map<std::string, clang::FileID> filesByPath;
};

class ASTStorage {
public:
  ASTStorage(const MullDiagnostics &diagnostics, const std::string &cxxCompilationDatabasePath,
             const std::string &cxxCompilationFlags,
             const std::unordered_map<std::string, std::string> &bitcodeCompilationFlags);

  ASTUnitWrapper *findAST(const mull::SourceLocation &sourceLocation);
  ASTUnitWrapper *findAST(const std::string &sourceFile);

  void setAST(const std::string &sourceFile, std::unique_ptr<ASTUnitWrapper> astUnit);

private:
  const MullDiagnostics &diagnostics;
  CompilationDatabase compilationDatabase;
  std::unordered_map<std::string, std::unique_ptr<ASTUnitWrapper>> astUnits;
};

} // namespace mull
