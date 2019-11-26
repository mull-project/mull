#pragma once

#include "mull/Mutators/CXX/TrivialCXXMutator.h"

namespace mull {
namespace cxx {

#pragma mark - Shifts

class LShiftToRShift : public TrivialCXXMutator {
public:
  static const std::string ID;
  LShiftToRShift();
};

class LShiftAssignToRShiftAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  LShiftAssignToRShiftAssign();
};

class RShiftToLShift : public TrivialCXXMutator {
public:
  static const std::string ID;
  RShiftToLShift();
};

class RShiftAssignToLShiftAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  RShiftAssignToLShiftAssign();
};

#pragma mark - Bit operations

class BitwiseOrToAnd : public TrivialCXXMutator {
public:
  static const std::string ID;
  BitwiseOrToAnd();
};

class OrAssignToAndAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  OrAssignToAndAssign();
};

class BitwiseAndToOr : public TrivialCXXMutator {
public:
  static const std::string ID;
  BitwiseAndToOr();
};

class AndAssignToOrAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  AndAssignToOrAssign();
};

class XorToOr : public TrivialCXXMutator {
public:
  static const std::string ID;
  XorToOr();
};

class XorAssignToOrAssign : public TrivialCXXMutator {
public:
  static const std::string ID;
  XorAssignToOrAssign();
};

} // namespace cxx
} // namespace mull