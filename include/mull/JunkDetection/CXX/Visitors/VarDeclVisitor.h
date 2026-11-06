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
      /// The IR also stores constants into structs and pointers, `auto t = 74` is not valid there
      if (!takesNumber(decl->getType())) {
        return false;
      }
      matchingExpression = decl->getInit();
      if (auto initList = clang::dyn_cast<clang::InitListExpr>(matchingExpression)) {
        /// `T a{}`: no value was picked, so there is nothing to replace
        if (initList->getNumInits() == 0) {
          matchingExpression = nullptr;
        }
        /// `T a{b}`: replace `b`, keeping the braces
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
  /// Dependent types are kept: the instantiations are not visible here
  bool takesNumber(clang::QualType type) const {
    if (type->isDependentType()) {
      return true;
    }
    if (type->isEnumeralType()) {
      return !parameters.astContext.getLangOpts().CPlusPlus;
    }
    return type->isArithmeticType();
  }

  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
};

} // namespace cxx
} // namespace mull
