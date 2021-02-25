#pragma once

#include "mull/Mutators/MutatorKind.h"

namespace mull {

struct MutationPointSourceInfo {
  int beginLine = -1;
  int beginColumn = -1;
  int endLine = -1;
  int endColumn = -1;
};

class ASTStorage;
class MutationPoint;
class Diagnostics;
struct SourceLocation;

class SourceInfoProvider {
public:
  virtual MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics,
                                                const SourceLocation &sourceLocation,
                                                MutatorKind mutatorKind) = 0;
};

class ASTSourceInfoProvider : public SourceInfoProvider {
public:
  ~ASTSourceInfoProvider() = default;
  explicit ASTSourceInfoProvider(ASTStorage &astStorage);
  MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics,
                                        const SourceLocation &sourceLocation,
                                        MutatorKind mutatorKind) override;

private:
  ASTStorage &astStorage;
};

} // namespace mull
