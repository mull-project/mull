#pragma once

#include "mull/Mutators/DescribableMutation.h"

#include <cstdint>
#include <irm/irm.h>
#include <optional>
#include <string>

namespace llvm {
class Instruction;
class StoreInst;
class Value;
} // namespace llvm

namespace mull {

namespace cxx {

/// Replaces the value of an integer store with a constant.
///
/// Behaves like irm::StoreIntReplacement, except for stores of the constants 0
/// and 1 into a location that is read back as a boolean: those are negated
/// instead of set to \c value.
///
/// A bool is stored as a byte but read back through `trunc iN %v to i1`, which
/// keeps only the low bit. Replacing `false` with an arbitrary even constant
/// such as 42 therefore leaves the program behaving exactly as before - an
/// equivalent mutant that no test can kill. Negating the stored constant
/// instead yields a killable mutant in both directions.
///
/// The `trunc` is what distinguishes a bool from the other byte-sized types: at
/// IR level `bool b = false` and `char c = 0` both emit `store i8 0`, but only
/// a bool is truncated to i1 when read - a char is compared with `icmp`. When
/// no such read is found the store keeps the \c value replacement, so types
/// this cannot classify are left exactly as they were.
class IntegerStoreReplacement : public irm::IRMutation, public DescribableMutation {
public:
  /// \p spelling is how a stored constant reads in a report, with `{}` standing
  /// for the value: "= {};" renders as "= 42;" or, for a negated bool, "= 0;".
  IntegerStoreReplacement(int64_t value, std::string spelling);

  bool canMutate(llvm::Instruction *instruction) override;
  void mutate(llvm::Instruction *instruction) override;

  std::optional<std::string>
  describeReplacement(const llvm::Instruction *instruction) const override;

  /// The boolean value \p instruction stores once mutated, or nullopt when it is
  /// not a store this negates.
  static std::optional<bool> negatedBooleanValue(const llvm::Instruction *instruction);

private:
  /// The constant this stores into \p instruction, or nullopt when it does not
  /// mutate it.
  std::optional<int64_t> replacementValue(const llvm::Instruction *instruction) const;

  int64_t value;
  std::string spelling;
};

} // namespace cxx
} // namespace mull
