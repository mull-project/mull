#pragma once

#include "JunkDetection/CXX/CompilationDatabase.h"

#include <clang/Frontend/ASTUnit.h>

#include <map>
#include <string>
#include <mutex>

namespace mull {

class MutationPoint;

class ASTStorage {
public:
  ASTStorage(const std::string &cxxCompilationDatabasePath,
             const std::string &cxxCompilationFlags);

  const clang::ASTUnit *findAST(const MutationPoint *point);
  const clang::FileEntry *findFileEntry(const clang::ASTUnit *ast,
                                        const MutationPoint *point);

private:
  std::vector<std::string> commandLineArguments(const std::string &sourceFile);

  std::mutex mutex;

  CompilationDatabase compilationDatabase;
  std::map<std::string, std::unique_ptr<clang::ASTUnit>> astUnits;
};

} // namespace mull