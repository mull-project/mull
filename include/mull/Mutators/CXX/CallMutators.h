#pragma once

#include "mull/Mutators/CXX/TrivialCXXMutator.h"

namespace mull {
namespace cxx {

class RemoveVoidCall : public TrivialCXXMutator {
public:
  static std::string ID();
  RemoveVoidCall();
};

class ReplaceScalarCallZero : public TrivialCXXMutator {
public:
  static std::string ID();
  ReplaceScalarCallZero();
};

class ReplaceScalarCallNonZero : public TrivialCXXMutator {
public:
  static std::string ID();
  ReplaceScalarCallNonZero();
};

} // namespace cxx
} // namespace mull
