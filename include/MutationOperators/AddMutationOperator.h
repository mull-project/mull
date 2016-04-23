#pragma mark

#include "MutationOperators/MutationOperator.h"

namespace Mutang {

  class AddMutationOperator : public MutationOperator {
  public:
    virtual bool canBeApplied(llvm::Value &V);
  };

}
