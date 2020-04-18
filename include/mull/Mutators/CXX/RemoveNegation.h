#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class RemoveNegation : public TrivialCXXMutator {
public:
  static std::string ID();
  RemoveNegation();
};

} // namespace cxx

} // namespace mull