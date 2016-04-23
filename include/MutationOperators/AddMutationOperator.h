#pragma mark

#include "MutationOperators/MutationOperator.h"

namespace Mutang {

  class AddMutationOperator : MutationOperator {
  public:
    virtual bool canBeApplied(llvm::Value &V);
  };

}
