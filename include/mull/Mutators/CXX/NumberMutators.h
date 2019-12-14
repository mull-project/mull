#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class NumberAssignConst : public TrivialCXXMutator {
public:
  static std::string ID();
  NumberAssignConst();
};

class NumberInitConst : public TrivialCXXMutator {
public:
  static std::string ID();
  NumberInitConst();
};

}
} // namespace mull