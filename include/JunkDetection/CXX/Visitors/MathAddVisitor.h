#pragma once

#include "JunkDetection/CXX/Visitors/InstructionRangeVisitor.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

class MathAddVisitor : public clang::RecursiveASTVisitor<MathAddVisitor> {
public:
  MathAddVisitor(const clang::SourceManager &sourceManager,
                 const clang::SourceLocation &sourceLocation);

  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  bool foundMutant();

private:
  InstructionRangeVisitor visitor;
};

} // namespace mull
