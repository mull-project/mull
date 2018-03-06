#pragma once

#include "JunkDetection/JunkDetector.h"

#include <map>
#include <string>
#include <clang-c/Index.h>

#include <clang/Tooling/CompilationDatabase.h>

namespace mull {

class MutationPoint;

struct PhysicalAddress {
  std::string directory;
  std::string filepath;
  uint32_t line;
  uint32_t column;
  PhysicalAddress();
  bool valid();
};

class CXXJunkDetector : public JunkDetector {
public:
  CXXJunkDetector();
  ~CXXJunkDetector();

  bool isJunk(MutationPoint *point) override;
private:
  std::pair<CXCursor, CXSourceLocation> cursorAndLocation(PhysicalAddress &address, MutationPoint *point);
  CXTranslationUnit translationUnit(const std::string &directory,
                                    const std::string &sourceFile);

  bool isJunkBoundary(CXCursor cursor, CXSourceLocation location, PhysicalAddress &address, MutationPoint *point);
  bool isJunkMathAdd(CXCursor cursor, CXSourceLocation location, PhysicalAddress &address, MutationPoint *point);
  bool isJunkNegate(CXCursor cursor, CXSourceLocation location, PhysicalAddress &address, MutationPoint *point);
  bool isJunkRemoveVoid(CXCursor cursor, CXSourceLocation location, PhysicalAddress &address, MutationPoint *point);

  CXIndex index;
  std::map<std::string, CXTranslationUnit> units;
  std::unique_ptr<clang::tooling::CompilationDatabase> compdb;
};

}
