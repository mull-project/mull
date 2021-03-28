#pragma once

namespace mull {

struct MutationPointSourceInfo {
  int beginLine = -1;
  int beginColumn = -1;
  int endLine = -1;
  int endColumn = -1;
};

class ASTStorage;
class Mutant;
class Diagnostics;

class SourceInfoProvider {
public:
  virtual MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics, Mutant *mutant) = 0;
};

class ASTSourceInfoProvider : public SourceInfoProvider {
public:
  ~ASTSourceInfoProvider() = default;
  explicit ASTSourceInfoProvider(ASTStorage &astStorage);
  MutationPointSourceInfo getSourceInfo(Diagnostics &diagnostics, Mutant *mutant) override;

private:
  ASTStorage &astStorage;
};

} // namespace mull
