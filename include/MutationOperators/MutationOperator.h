#pragma once

#include "MutationOperators/MutationOperatorFilter.h"

#include <string>
#include <vector>

namespace llvm {
  class Function;
  class Value;
  class Module;
}

namespace mull {

class Context;
class MutationPoint;
class MutationPointAddress;
class MutationOperatorFilter;

class MutationOperator {
public:
  virtual std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                         llvm::Function *function,
                                                         MutationOperatorFilter &filter) = 0;

  /// FIXME: Renmae to 'getUniqueIdentifier'
  virtual std::string uniqueID() = 0;
  virtual std::string uniqueID() const = 0;

  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) = 0;
  virtual ~MutationOperator() {}
};

}
