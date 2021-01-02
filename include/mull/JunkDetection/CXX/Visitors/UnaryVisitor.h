#pragma once

#include "VisitorParameters.h"
#include <cassert>
#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

namespace cxx {

class UnaryVisitor : public clang::RecursiveASTVisitor<UnaryVisitor> {
public:
  static clang::Expr *findMutant(const VisitorParameters &parameters,
                                 clang::UnaryOperator::Opcode opcode, clang::Decl *decl);
  UnaryVisitor(const VisitorParameters &parameters, clang::UnaryOperator::Opcode opcode);
  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator);
  clang::Expr *foundMutant();

private:
  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
  clang::UnaryOperator::Opcode opcode;
};

} // namespace cxx

} // namespace mull
