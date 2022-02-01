#pragma once

#include "mull/Filters/Filter.h"
#include <string>

namespace mull {

class MutationPoint;

class MutationFilter : virtual public Filter {
public:
  virtual bool shouldSkip(MutationPoint *point) = 0;
  virtual std::string name() = 0;
  ~MutationFilter() override = default;
};

} // namespace mull
