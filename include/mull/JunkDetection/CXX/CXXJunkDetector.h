#pragma once

#include "ASTStorage.h"
#include "mull/JunkDetection/JunkDetector.h"
struct MullDiagnostics;
namespace mull {

class MutationPoint;

class CXXJunkDetector : public JunkDetector {
public:
  CXXJunkDetector(const MullDiagnostics &diagnostics, ASTStorage &storage);
  ~CXXJunkDetector() override = default;

  bool isJunk(MutationPoint *point) override;

private:
  const MullDiagnostics &diagnostics;
  ASTStorage &astStorage;
};

} // namespace mull
