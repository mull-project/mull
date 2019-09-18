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

class SubToAdd : public TrivialCXXMutator {
public:
  static const std::string ID;
  SubToAdd();
};

class SubAssignToAddAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  SubAssignToAddAssign();
};

class PostDecToPostInc : public TrivialCXXMutator {
public:
  static const std::string ID;
  PostDecToPostInc();
};

class PreDecToPreInc : public TrivialCXXMutator {
public:
  static const std::string ID;
  PreDecToPreInc();
};

class MulToDiv : public TrivialCXXMutator {
public:
  static const std::string ID;
  MulToDiv();
};

class MulAssignToDivAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  MulAssignToDivAssign();
};

} // namespace cxx

} // namespace mull