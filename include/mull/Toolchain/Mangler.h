#pragma once

#include <llvm/IR/DataLayout.h>

#include <string>

namespace mull {

class Mangler {
public:
  explicit Mangler(const llvm::DataLayout &dataLayout);
  /// We use LLVM Mangler class for low-level mangling: '_' prefixing.
  /// Examples:
  /// Mac OS:
  /// _ZN7testing14InitGoogleTestEPiPPc -> __ZN7testing14InitGoogleTestEPiPPc
  /// On Linux it has no effect:
  /// _ZN7testing14InitGoogleTestEPiPPc -> _ZN7testing14InitGoogleTestEPiPPc
  std::string getNameWithPrefix(const std::string &name);

private:
  const llvm::DataLayout dataLayout;
};

} // namespace mull
