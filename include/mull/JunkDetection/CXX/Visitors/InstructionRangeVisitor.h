#pragma once

#include "VisitorParameters.h"

#include <clang/Basic/SourceLocation.h>

namespace clang {
class SourceManager;
}

namespace mull {

class InstructionRangeVisitor {
public:
  InstructionRangeVisitor(const VisitorParameters &parameters);
  void visitRangeWithLocation(const clang::SourceRange &range);
  bool foundRange();

private:
  const clang::SourceManager &sourceManager;
  const clang::SourceLocation &sourceLocation;
  clang::SourceRange sourceRange;
};

} // namespace mull
