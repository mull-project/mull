#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class ReplaceAssignmentVisitor : public clang::RecursiveASTVisitor<ReplaceAssignmentVisitor> {
public:
  ReplaceAssignmentVisitor(const VisitorParameters &parameters);

  bool VisitBinAssign(clang::BinaryOperator *binaryOperator);
  clang::Expr *foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull
