#include "mull/MutationFilters/NoDebugInfoFilter.h"

#include "mull/MutationPoint.h"

using namespace mull;

bool NoDebugInfoFilter::shouldSkip(MutationPoint *point) {
  return point->getSourceLocation().isNull();
}

std::string NoDebugInfoFilter::name() { return "no debug info"; }
