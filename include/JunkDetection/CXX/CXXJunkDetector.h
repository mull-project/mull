#pragma once

#include "JunkDetection/JunkDetector.h"

#include <map>
#include <string>
#include <clang-c/Index.h>

namespace mull {

class MutationPoint;

struct PhysicalAddress {
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
  std::pair<CXCursor, CXSourceLocation> cursorAndLocation(PhysicalAddress &address);

  bool isJunkBoundary(CXCursor cursor, CXSourceLocation location, PhysicalAddress &address, MutationPoint *point);

  CXIndex index;
  std::map<std::string, CXTranslationUnit> units;
};

}
