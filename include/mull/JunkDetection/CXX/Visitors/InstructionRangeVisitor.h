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
  void visitRangeWithASTExpr(const clang::Stmt *stmt);
  const clang::Stmt *getMatchingASTNode();

private:
  const clang::SourceManager &sourceManager;
  const clang::SourceLocation &sourceLocation;
  const clang::Stmt *bestMatchingASTNode;
};

} // namespace mull
