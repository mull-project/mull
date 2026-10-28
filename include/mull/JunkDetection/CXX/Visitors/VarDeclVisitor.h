#pragma once

#include <cassert>
#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

namespace cxx {

class VarDeclVisitor : public clang::RecursiveASTVisitor<VarDeclVisitor> {
public:
  explicit VarDeclVisitor(const VisitorParameters &parameters)
      : parameters(parameters), matchingExpression(nullptr) {}

  bool VisitVarDecl(clang::VarDecl *decl) {
    if (decl->hasDefinition() == clang::VarDecl::DeclarationOnly) {
      return true;
    }
    if (!decl->hasInit()) {
      return true;
    }

    if (decl->getType().isConstQualified()) {
      return true;
    }

    if (decl->getLocation() == parameters.sourceLocation) {
      assert(!matchingExpression && "Already found");
      matchingExpression = decl->getInit();
      /// `T a{b}`: replace `b`, keeping the braces
      if (auto initList = clang::dyn_cast<clang::InitListExpr>(matchingExpression)) {
        if (initList->getNumInits() == 1) {
          matchingExpression = initList->getInit(0);
        }
      }
      return false;
    }
    return true;
  }

  clang::Expr *foundMutant() {
    return matchingExpression;
  }

private:
  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
};

} // namespace cxx
} // namespace mull
