#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class LessThanToLessOrEqual : public TrivialCXXMutator {
public:
  static std::string ID();
  LessThanToLessOrEqual();
};

class LessOrEqualToLessThan : public TrivialCXXMutator {
public:
  static std::string ID();
  LessOrEqualToLessThan();
};

class GreaterThanToGreaterOrEqual : public TrivialCXXMutator {
public:
  static std::string ID();
  GreaterThanToGreaterOrEqual();
};

class GreaterOrEqualToGreaterThan : public TrivialCXXMutator {
public:
  static std::string ID();
  GreaterOrEqualToGreaterThan();
};

class EqualToNotEqual : public TrivialCXXMutator {
public:
  static std::string ID();
  EqualToNotEqual();
};

class NotEqualToEqual : public TrivialCXXMutator {
public:
  static std::string ID();
  NotEqualToEqual();
};

class GreaterThanToLessOrEqual : public TrivialCXXMutator {
public:
  static std::string ID();
  GreaterThanToLessOrEqual();
};

class GreaterOrEqualToLessThan : public TrivialCXXMutator {
public:
  static std::string ID();
  GreaterOrEqualToLessThan();
};

class LessThanToGreaterOrEqual : public TrivialCXXMutator {
public:
  static std::string ID();
  LessThanToGreaterOrEqual();
};

class LessOrEqualToGreaterThan : public TrivialCXXMutator {
public:
  static std::string ID();
  LessOrEqualToGreaterThan();
};

} // namespace cxx

} // namespace mull