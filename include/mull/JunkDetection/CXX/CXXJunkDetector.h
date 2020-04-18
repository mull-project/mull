#pragma once

#include "ASTStorage.h"
#include "mull/JunkDetection/JunkDetector.h"

namespace mull {

class MutationPoint;

class CXXJunkDetector : public JunkDetector {
public:
  explicit CXXJunkDetector(ASTStorage &storage);
  ~CXXJunkDetector() override = default;

  bool isJunk(MutationPoint *point) override;

private:
  ASTStorage &astStorage;
};

} // namespace mull
