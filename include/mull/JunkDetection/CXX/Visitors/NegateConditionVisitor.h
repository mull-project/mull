#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class NegateConditionVisitor
    : public clang::RecursiveASTVisitor<NegateConditionVisitor> {
public:
  NegateConditionVisitor(const VisitorParameters &parameters);

  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  const clang::Stmt *foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull