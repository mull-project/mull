//
// Created by Stanislav Pankevich on 23.04.21.
//

#ifndef CLANG_PLUGIN_TEST_SRC_MULLTREETRANSFORM_H
#define CLANG_PLUGIN_TEST_SRC_MULLTREETRANSFORM_H

#include "clang/Sema/TreeTransform.h"

class MullTreeTransform : public clang::TreeTransform<MullTreeTransform> {
public:
  explicit MullTreeTransform(clang::Sema &sema) : TreeTransform(sema) {
  }

  bool AlwaysRebuild() {
    return true;
  }  // needed for cloning the lhs
  //
  bool ReplacingOriginal() {
    return true;
  }

  clang::ExprResult TransformBinaryOperator(clang::BinaryOperator *E) {
//      clang::ExprResult LHS = getDerived().TransformExpr(E->getLHS());
//      if (LHS.isInvalid())
//        return clang::ExprError();
//
//      clang::ExprResult RHS = getDerived().TransformExpr(E->getRHS());
//      if (RHS.isInvalid())
//        return clang::ExprError();
//
//      if (!getDerived().AlwaysRebuild() && LHS.get() == E->getLHS() &&
//        RHS.get() == E->getRHS())
//        return E;
    //
    //    Sema::FPContractStateRAII FPContractState(getSema());
    //    getSema().FPFeatures = E->getFPFeatures();
    //
    return getDerived().RebuildBinaryOperator(
      E->getOperatorLoc(),
      E->getOpcode(),
      E->getLHS(),
      E->getRHS());
  };
};
#endif  // CLANG_PLUGIN_TEST_SRC_MULLTREETRANSFORM_H
