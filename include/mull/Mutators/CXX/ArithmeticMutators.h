#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class AddToSub : public TrivialCXXMutator {
public:
  static const std::string ID;
  AddToSub();
};

class AddAssignToSubAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  AddAssignToSubAssign();
};

class PostIncToPostDec : public TrivialCXXMutator {
public:
  static const std::string ID;
  PostIncToPostDec();
};

class PreIncToPreDec : public TrivialCXXMutator {
public:
  static const std::string ID;
  PreIncToPreDec();
};

} // namespace cxx

} // namespace mull