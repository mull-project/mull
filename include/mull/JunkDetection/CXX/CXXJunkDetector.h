#pragma once

#include "ASTStorage.h"
#include "mull/JunkDetection/JunkDetector.h"

namespace mull {

class MutationPoint;
struct JunkDetectionConfig;

class CXXJunkDetector : public JunkDetector {
public:
  explicit CXXJunkDetector(JunkDetectionConfig &config);
  ~CXXJunkDetector() override = default;

  bool isJunk(MutationPoint *point) override;

private:
  ASTStorage astStorage;
};

} // namespace mull
