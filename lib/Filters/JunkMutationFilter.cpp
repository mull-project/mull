#include "mull/Filters/JunkMutationFilter.h"

#include "mull/JunkDetection/JunkDetector.h"

using namespace mull;

JunkMutationFilter::JunkMutationFilter(JunkDetector &junkDetector)
    : junkDetector(junkDetector) {}

bool JunkMutationFilter::shouldSkip(MutationPoint *point) {
  return junkDetector.isJunk(point);
}

std::string JunkMutationFilter::name() { return "junk"; }
