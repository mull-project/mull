#pragma once

#include "JunkDetection/JunkDetector.h"
#include "JunkDetection/CXX/ASTStorage.h"
#include "SourceLocation.h"

namespace mull {

class MutationPoint;
struct JunkDetectionConfig;

class CXXJunkDetector : public JunkDetector {
public:
  explicit CXXJunkDetector(JunkDetectionConfig &config);
  ~CXXJunkDetector() = default;

  bool isJunk(MutationPoint *point) override;
private:
  bool isJunkBoundaryConditional(MutationPoint *point,
                                 SourceLocation &mutantLocation);
  bool isJunkMathAdd(MutationPoint *point, SourceLocation &mutantLocation);
  bool isJunkMathSub(MutationPoint *point, SourceLocation &mutantLocation);
  bool isJunkRemoveVoidFunction(MutationPoint *point,
                                SourceLocation &mutantLocation);
  bool isJunkNegateCondition(MutationPoint *point,
                             SourceLocation &mutantLocation);

  ASTStorage astStorage;
};

}
