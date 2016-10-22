#pragma mark

#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/Instructions.h"

#include <vector>

namespace Mutang {

  class MutationPoint;
  class MutationPointAddress;

  class NegateConditionMutationOperator : public MutationOperator {

  public:
    static llvm::CmpInst::Predicate negatedCmpInstPredicate(llvm::CmpInst::Predicate predicate);
    std::vector<MutationPoint *> getMutationPoints(llvm::Function *function) override;

    bool canBeApplied(llvm::Value &V) override;
    llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
    llvm::Value *revertMutation(llvm::Value &Value) override;
  };
}
