#pragma once

#include "JunkDetection/JunkDetector.h"
#include "JunkDetection/CXX/ASTStorage.h"

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

}
