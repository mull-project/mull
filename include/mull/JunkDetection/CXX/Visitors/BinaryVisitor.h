#pragma once

#include "VisitorParameters.h"
#include <cassert>
#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

namespace cxx {

class BinaryVisitor : public clang::RecursiveASTVisitor<BinaryVisitor> {
public:
  static clang::Expr *findMutant(const VisitorParameters &parameters, clang::Decl *decl,
                                 clang::BinaryOperator::Opcode opcode);
  BinaryVisitor(const VisitorParameters &parameters, clang::BinaryOperator::Opcode opcode);
  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator);
  clang::Expr *foundMutant();

private:
  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
  clang::BinaryOperator::Opcode opcode;
};

} // namespace cxx

} // namespace mull
