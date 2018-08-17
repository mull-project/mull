#pragma once

#include "JunkDetection/JunkDetector.h"
#include "SourceLocation.h"

#include <map>
#include <string>
#include <mutex>
#include <clang-c/Index.h>

#include <clang/Tooling/CompilationDatabase.h>

namespace mull {

class MutationPoint;
struct JunkDetectionConfig;

class CXXJunkDetector : public JunkDetector {
public:
  CXXJunkDetector(JunkDetectionConfig &config);
  ~CXXJunkDetector();

  bool isJunk(MutationPoint *point) override;
private:
  std::mutex mutex;
  std::pair<CXCursor, CXSourceLocation> cursorAndLocation(MutationPoint *point);
  CXTranslationUnit translationUnit(const SourceLocation &location, const std::string &sourceFile);

  bool isJunkBoundary(CXCursor cursor, CXSourceLocation location, MutationPoint *point);
  bool isJunkMathAdd(CXCursor cursor, CXSourceLocation location, MutationPoint *point);
  bool isJunkNegate(CXCursor cursor, CXSourceLocation location, MutationPoint *point);
  bool isJunkRemoveVoid(CXCursor cursor, CXSourceLocation location, MutationPoint *point);

  CXIndex index;
  std::map<std::string, CXTranslationUnit> units;
  std::unique_ptr<clang::tooling::CompilationDatabase> compdb;
  std::vector<std::string> compilationFlags;
};

}
