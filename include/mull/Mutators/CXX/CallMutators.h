#pragma once

#include "mull/Mutators/CXX/TrivialCXXMutator.h"

namespace mull {
namespace cxx {

class RemoveVoidCall : public TrivialCXXMutator {
public:
  static std::string ID();
  RemoveVoidCall();
};

class ReplaceScalarCall : public TrivialCXXMutator {
public:
  static std::string ID();
  ReplaceScalarCall();
};

} // namespace cxx
} // namespace mull