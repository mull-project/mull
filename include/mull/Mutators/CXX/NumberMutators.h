#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class NumberAssignConst : public TrivialCXXMutator {
public:
  static const std::string ID;
  NumberAssignConst();
};

class NumberInitConst : public TrivialCXXMutator {
public:
  static const std::string ID;
  NumberInitConst();
};

}
} // namespace mull