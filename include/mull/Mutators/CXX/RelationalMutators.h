#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class LessThanToLessOrEqual : public TrivialCXXMutator {
public:
  static const std::string ID;
  LessThanToLessOrEqual();
};

class LessOrEqualToLessThan : public TrivialCXXMutator {
public:
  static const std::string ID;
  LessOrEqualToLessThan();
};

class GreaterThanToGreaterOrEqual : public TrivialCXXMutator {
public:
  static const std::string ID;
  GreaterThanToGreaterOrEqual();
};

class GreaterOrEqualToGreaterThan : public TrivialCXXMutator {
public:
  static const std::string ID;
  GreaterOrEqualToGreaterThan();
};

} // namespace cxx

} // namespace mull