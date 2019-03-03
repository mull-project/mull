#pragma once

#include "JunkDetection/CXX/Visitors/InstructionRangeVisitor.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class ConditionalsBoundaryVisitor
    : public clang::RecursiveASTVisitor<ConditionalsBoundaryVisitor> {
public:
  ConditionalsBoundaryVisitor(const clang::SourceManager &sourceManager,
                              const clang::SourceLocation &sourceLocation);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull
