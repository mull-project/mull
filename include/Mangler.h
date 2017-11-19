#pragma once

#include <string>

namespace llvm {
  class DataLayout;
}

namespace mull {

class Mangler {

public:

  /// We use LLVM Mangler class for low-level mangling: '_' prefixing.
  /// Examples:
  /// Mac OS:
  /// _ZN7testing14InitGoogleTestEPiPPc -> __ZN7testing14InitGoogleTestEPiPPc
  /// On Linux it has no effect:
  /// _ZN7testing14InitGoogleTestEPiPPc -> _ZN7testing14InitGoogleTestEPiPPc
  std::string getNameWithPrefix(const std::string &name,
                                const llvm::DataLayout &dataLayout);
  
};

}
