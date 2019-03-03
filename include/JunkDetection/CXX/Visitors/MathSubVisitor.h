#pragma once

#include "JunkDetection/CXX/Visitors/InstructionRangeVisitor.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class MathSubVisitor : public clang::RecursiveASTVisitor<MathSubVisitor> {
public:
  MathSubVisitor(const clang::SourceManager &sourceManager,
                 const clang::SourceLocation &sourceLocation);

  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  bool foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull
