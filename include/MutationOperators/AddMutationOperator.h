#pragma mark

#include "MutationOperators/MutationOperator.h"

#include <vector>

namespace Mutang {

class MutationPoint;
class MutationPointAddress;

class AddMutationOperator : public MutationOperator {
public:
  std::vector<MutationPoint *> getMutationPoints(llvm::Function *function) override;

  std::string uniqueID() override {
    return "add_mutation_operator";
  }
  std::string uniqueID() const override {
    return "add_mutation_operator";
  }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
  llvm::Value *revertMutation(llvm::Value &Value) override;
};

}
