#pragma once

#include "MutationFilter.h"

namespace mull {

class NoDebugInfoFilter : public MutationFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  std::string name() override;

private:
};

}