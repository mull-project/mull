#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class ConditionalsBoundaryVisitor
    : public clang::RecursiveASTVisitor<ConditionalsBoundaryVisitor> {
public:
  ConditionalsBoundaryVisitor(const VisitorParameters &parameters);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull
