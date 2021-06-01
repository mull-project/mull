#pragma once

#include "ASTStorage.h"
#include "mull/JunkDetection/JunkDetector.h"

namespace mull {

class MutationPoint;

class CXXJunkDetector : public JunkDetector {
public:
  CXXJunkDetector(Diagnostics &diagnostics, ASTStorage &storage);
  ~CXXJunkDetector() override = default;

  bool isJunk(MutationPoint *point) override;

private:
  Diagnostics &diagnostics;
  ASTStorage &astStorage;
};

} // namespace mull
