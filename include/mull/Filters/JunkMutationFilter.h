#pragma once

#include "MutationFilter.h"

namespace mull {

class JunkDetector;
class MutationPoint;

class JunkMutationFilter : public MutationFilter {
public:
  explicit JunkMutationFilter(JunkDetector &junkDetector);
  bool shouldSkip(MutationPoint *point) override;
  std::string name() override;

private:
  JunkDetector &junkDetector;
};

} // namespace mull