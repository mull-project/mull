#pragma once

#include "MutationPointFilter.h"

namespace mull {

class JunkDetector;
class MutationPoint;

class JunkMutationFilter : public MutationPointFilter {
public:
  explicit JunkMutationFilter(JunkDetector &junkDetector);
  bool shouldSkip(MutationPoint *point) override;
  std::string name() override;

private:
  JunkDetector &junkDetector;
};

} // namespace mull
