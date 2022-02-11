#pragma once

#include "mull/Filters/Filter.h"
#include <string>

namespace mull {

class Mutant;

class MutantFilter : virtual public Filter {
public:
  virtual bool shouldSkip(Mutant *mutant) = 0;
  virtual std::string name() = 0;
  ~MutantFilter() override = default;
};

} // namespace mull
