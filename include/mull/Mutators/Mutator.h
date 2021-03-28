#pragma once

#include "mull/Mutators/MutatorKind.h"

#include <string>
#include <vector>

namespace llvm {
class Function;
class Value;
class Module;
class Instruction;
} // namespace llvm

namespace irm {
class IRMutation;
}

namespace mull {

class Bitcode;
class MutationPoint;
class MutationPointAddress;
class FunctionUnderTest;
struct SourceLocation;

class Mutator {
public:
  virtual std::string getUniqueIdentifier() = 0;
  virtual std::string getUniqueIdentifier() const = 0;
  virtual MutatorKind mutatorKind() = 0;

  virtual std::string getDescription() const = 0;
  virtual std::string getDiagnostics() const = 0;
  virtual std::string getReplacement() const = 0;

  virtual void applyMutation(llvm::Function *function, const MutationPointAddress &address,
                             irm::IRMutation *lowLevelMutation) = 0;
  virtual std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                                    const FunctionUnderTest &function) = 0;

  virtual ~Mutator() = default;
};

} // namespace mull
