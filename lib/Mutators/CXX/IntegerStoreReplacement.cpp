#include "mull/Mutators/CXX/IntegerStoreReplacement.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <utility>

using namespace mull;
using namespace mull::cxx;

/// Whether any load of \p pointer is truncated to i1, i.e. whether the location
/// holds a value only the low bit of which is ever read.
static bool readAsBoolean(const llvm::Value *pointer) {
  for (const llvm::User *user : pointer->users()) {
    const auto *load = llvm::dyn_cast<llvm::LoadInst>(user);
    if (load == nullptr) {
      continue;
    }
    for (const llvm::User *loadUser : load->users()) {
      const auto *trunc = llvm::dyn_cast<llvm::TruncInst>(loadUser);
      if (trunc != nullptr && trunc->getDestTy()->isIntegerTy(1)) {
        return true;
      }
    }
  }
  return false;
}

/// The integer store \p instruction is, or nullptr when it is not one.
static const llvm::StoreInst *integerStore(const llvm::Instruction *instruction) {
  const auto *store = llvm::dyn_cast_or_null<llvm::StoreInst>(instruction);
  if (store == nullptr || !store->getValueOperand()->getType()->isIntegerTy()) {
    return nullptr;
  }
  return store;
}

/// \p spelling with `{}` replaced by \p value.
static std::string render(const std::string &spelling, int64_t value) {
  const std::string placeholder = "{}";
  std::string rendered = spelling;
  const size_t position = rendered.find(placeholder);
  if (position != std::string::npos) {
    rendered.replace(position, placeholder.length(), std::to_string(value));
  }
  return rendered;
}

IntegerStoreReplacement::IntegerStoreReplacement(int64_t value, std::string spelling)
    : value(value), spelling(std::move(spelling)) {}

bool IntegerStoreReplacement::canMutate(llvm::Instruction *instruction) {
  return integerStore(instruction) != nullptr;
}

void IntegerStoreReplacement::mutate(llvm::Instruction *instruction) {
  std::optional<int64_t> replacement = replacementValue(instruction);
  if (!replacement.has_value()) {
    return;
  }
  auto *store = llvm::cast<llvm::StoreInst>(instruction);
  store->setOperand(0, llvm::ConstantInt::get(store->getValueOperand()->getType(), *replacement));
}

std::optional<std::string>
IntegerStoreReplacement::describeReplacement(const llvm::Instruction *instruction) const {
  std::optional<int64_t> replacement = replacementValue(instruction);
  if (!replacement.has_value()) {
    return std::nullopt;
  }
  return render(spelling, *replacement);
}

std::optional<int64_t>
IntegerStoreReplacement::replacementValue(const llvm::Instruction *instruction) const {
  if (integerStore(instruction) == nullptr) {
    return std::nullopt;
  }
  if (std::optional<bool> negated = negatedBooleanValue(instruction)) {
    return *negated ? 1 : 0;
  }
  return value;
}

std::optional<bool>
IntegerStoreReplacement::negatedBooleanValue(const llvm::Instruction *instruction) {
  const llvm::StoreInst *store = integerStore(instruction);
  if (store == nullptr) {
    return std::nullopt;
  }
  const auto *constant = llvm::dyn_cast<llvm::ConstantInt>(store->getValueOperand());
  if (constant == nullptr) {
    return std::nullopt;
  }
  if (!constant->isZero() && !constant->isOne()) {
    return std::nullopt;
  }
  if (!readAsBoolean(store->getPointerOperand())) {
    return std::nullopt;
  }
  return constant->isZero();
}
