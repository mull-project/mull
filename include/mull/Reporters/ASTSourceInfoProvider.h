#pragma once

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

class SourceInfoProvider {
public:
  virtual MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics,
                                                MutationPoint *mutationPoint) = 0;
};

class ASTSourceInfoProvider : public SourceInfoProvider {
public:
  ~ASTSourceInfoProvider() = default;
  explicit ASTSourceInfoProvider(ASTStorage &astStorage);
  MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics,
                                        MutationPoint *mutationPoint) override;

private:
  ASTStorage &astStorage;
};

} // namespace mull
