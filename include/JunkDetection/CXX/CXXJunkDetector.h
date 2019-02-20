#pragma once

#include "JunkDetection/JunkDetector.h"
#include "SourceLocation.h"

#include <map>
#include <string>
#include <mutex>

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Frontend/ASTUnit.h>

namespace mull {

class MutationPoint;
struct JunkDetectionConfig;

class CXXJunkDetector : public JunkDetector {
public:
  explicit CXXJunkDetector(JunkDetectionConfig &config);
  ~CXXJunkDetector() = default;

  bool isJunk(MutationPoint *point) override;
private:
  std::mutex mutex;

  const clang::ASTUnit *findAST(const MutationPoint *point);
  const clang::FileEntry *findFileEntry(const clang::ASTUnit *ast,
                                        const MutationPoint *point);
  std::vector<std::string> commandLineArguments(const std::string &sourceFile);

  bool isJunkBoundaryConditional(MutationPoint *point,
                                 SourceLocation &mutantLocation);
  bool isJunkMathAdd(MutationPoint *point, SourceLocation &mutantLocation);
  bool isJunkMathSub(MutationPoint *point, SourceLocation &mutantLocation);

  std::unique_ptr<clang::tooling::CompilationDatabase> compdb;
  std::vector<std::string> compilationFlags;
  std::map<std::string, std::unique_ptr<clang::ASTUnit>> astUnits;
};

}
