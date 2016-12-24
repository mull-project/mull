#pragma mark

#include "MutationOperators/MutationOperator.h"

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;
class MutationOperatorFilter;

class AddMutationOperator : public MutationOperator {
public:
  std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                 llvm::Function *function,
                                                 MutationOperatorFilter &filter) override;

  std::string uniqueID() override {
    return "add_mutation_operator";
  }
  std::string uniqueID() const override {
    return "add_mutation_operator";
  }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
  llvm::Value *revertMutation(llvm::Value &Value) override __attribute__((unavailable));
};

}
