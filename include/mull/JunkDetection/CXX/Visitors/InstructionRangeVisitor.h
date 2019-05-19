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
  void visitRangeWithASTExpr(clang::Expr *expression);
  clang::Expr *getMatchingASTNode();

private:
  const clang::SourceManager &sourceManager;
  const clang::SourceLocation &sourceLocation;
  clang::Expr *bestMatchingASTNode;
};

} // namespace mull
