#pragma once

#include "Toolchain/ObjectCache.h"

namespace Mutang {
  class Toolchain {
    ObjectCache objectCache;
  public:
    Toolchain();

    ObjectCache &cache();
  };
}
