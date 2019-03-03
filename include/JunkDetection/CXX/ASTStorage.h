#pragma once

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/CompilationDatabase.h>

#include <map>
#include <string>
#include <mutex>

namespace mull {

class MutationPoint;

class ASTStorage {
public:
  ASTStorage(const std::string &cxxCompDBDirectory,
             const std::string &cxxCompilationFlags);

  const clang::ASTUnit *findAST(const MutationPoint *point);
  const clang::FileEntry *findFileEntry(const clang::ASTUnit *ast,
                                        const MutationPoint *point);

private:
  std::vector<std::string> commandLineArguments(const std::string &sourceFile);

  std::mutex mutex;

  std::unique_ptr<clang::tooling::CompilationDatabase> compdb;
  std::vector<std::string> compilationFlags;
  std::map<std::string, std::unique_ptr<clang::ASTUnit>> astUnits;
};

} // namespace mull