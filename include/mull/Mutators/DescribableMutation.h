#pragma once

#include <optional>
#include <string>

namespace llvm {
class Instruction;
}

namespace mull {

/// A low-level mutation whose report text depends on the instruction it
/// mutates, rather than being fixed for the whole mutator.
class DescribableMutation {
public:
  virtual ~DescribableMutation() = default;

  /// The report text for mutating \p instruction, or nullopt to fall back to
  /// the mutator's replacement.
  virtual std::optional<std::string>
  describeReplacement(const llvm::Instruction *instruction) const = 0;
};

} // namespace mull
