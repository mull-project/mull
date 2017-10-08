#pragma once

#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/Instructions.h"

#include <vector>

namespace mull {

  class NegateConditionMutationPoint;
  class MutationPointAddress;
  class MutationOperatorFilter;

  class NegateConditionMutationOperator : public MutationOperator {

    virtual bool canBeApplied(llvm::Value &V,
                              llvm::CmpInst::Predicate *outPredicate,
                              llvm::CmpInst::Predicate *outNegatedPredicate);

  public:
    static const std::string ID;

    static llvm::CmpInst::Predicate negatedCmpInstPredicate(llvm::CmpInst::Predicate predicate);
    std::vector<IMutationPoint *> getMutationPoints(const Context &context,
                                                    llvm::Function *function,
                                                    MutationOperatorFilter &filter) override;

    llvm::Value *applyMutation2(llvm::Module *M,
                                NegateConditionMutationPoint *mp);

    std::string uniqueID() override {
      return ID;
    }
    std::string uniqueID() const override {
      return ID;
    }

    bool canBeApplied(llvm::Value &V) override;
    llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
  };
}
