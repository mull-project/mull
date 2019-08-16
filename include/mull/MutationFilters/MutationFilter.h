#pragma once

#include <string>

namespace mull {

class MutationPoint;

class MutationFilter {
public:
  virtual bool shouldSkip(MutationPoint *point) = 0;
  virtual std::string name() = 0;
};

} // namespace mull
