#pragma once

#include "mull/Filters/Filter.h"
#include <string>

namespace mull {

class MutationPoint;

class MutationPointFilter : virtual public Filter {
public:
  virtual bool shouldSkip(MutationPoint *point) = 0;
  virtual std::string name() = 0;
  ~MutationPointFilter() override = default;
};

} // namespace mull
